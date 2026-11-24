#include "bug_report_screen.hpp"

#include "app_config.hpp"
#include "emulator.hpp"
#include "game_options.hpp"
#include "game_window.hpp"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <lucent/log.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <string_view>
#include <system_error>

namespace z3d::app {
namespace {

namespace bug = lucent::bug_report;

constexpr const char *kScreenFile = "screens.png";
constexpr const char *kStateFile = "state.state";
constexpr const char *kLogFile = "z3d_core.log";
// Text heights as shares of the window's height.
constexpr float kHeadingShare = 1.0F / 16.0F;
constexpr float kTextShare = 1.0F / 30.0F;
constexpr float kMargin = 0.04F;
// The held frame's share of the window's width.
constexpr float kFrameShare = 0.46F;
constexpr float kHintsTop = 0.92F;
constexpr float kFieldPad = 0.35F;

// The last UTF-8 character of `text` taken off.
void dropLast(std::string &text) {
  while (!text.empty()) {
    const auto byte = static_cast<unsigned char>(text.back());
    text.pop_back();
    if ((byte & 0xC0U) != 0x80U) {
      return;
    }
  }
}

Prompt keyPrompt(SDL_Scancode key, bool MenuPad::*button = nullptr) {
  Prompt prompt = button != nullptr ? PadInput::menuPrompt(button) : Prompt{};
  prompt.key = key;
  return prompt;
}

} // namespace

BugReportScreen::BugReportScreen(const MenuCanvas &canvas, GameWindow &window, const PadInput &pad,
                                 Emulator &emulator, const std::filesystem::path &userDir,
                                 const ReportedRun &run)
    : canvas_(canvas), window_(window), pad_(pad) {
  canvas_.upload(frame_, emulator.frame(), false);
  if (emulator.frame().height() > 0) {
    frameAspect_ = static_cast<float>(emulator.frame().width()) /
                   static_cast<float>(emulator.frame().height());
  }
  capture(emulator, userDir, run);
}

BugReportScreen::~BugReportScreen() {
  SDL_StopTextInput(window_.sdlWindow());
  SDL_DestroyTexture(frame_);
}

void BugReportScreen::capture(Emulator &emulator, const std::filesystem::path &userDir,
                              const ReportedRun &run) {
  const std::filesystem::path root = userDir / kFolder;
  bug::Draft::prune_abandoned(root);
  draft_ = bug::Draft::open(root, "z3d", std::chrono::system_clock::now(), problem_);
  if (!draft_) {
    lucent::error("report", "cannot start a report: {}", problem_);
    return;
  }
  std::string error;
  const auto attach = [&](const char *file, bug::AttachmentRole role, const char *label) {
    if (!draft_->attach(file, role, label, error)) {
      lucent::warn("report", "{}", error);
    }
  };
  if (const auto path = draft_->path_for(kScreenFile, error);
      path && emulator.frame().writePng(*path)) {
    attach(kScreenFile, bug::AttachmentRole::Screenshot, "Both screens as the game showed them");
  }
  if (const auto state = emulator.saveState()) {
    if (const auto path = draft_->path_for(kStateFile, error)) {
      std::ofstream(*path, std::ios::binary)
          .write(reinterpret_cast<const char *>(state->data()),
                 static_cast<std::streamsize>(state->size()));
      attach(kStateFile, bug::AttachmentRole::Reproduction, "The emulator's state at that frame");
    }
  }
  if (const auto path = draft_->path_for(kLogFile, error)) {
    std::error_code copied;
    std::filesystem::copy_file(userDir / "Azahar" / "log" / kLogFile, *path, copied);
    if (!copied) {
      attach(kLogFile, bug::AttachmentRole::Context, "The core's log of this run");
    }
  }
  draft_->add_fact("game", run.title);
  draft_->add_fact("title id", run.game.options.parent_path().filename().string());
  if (!run.game.inputs.empty()) {
    draft_->add_fact("set inputs", run.game.inputs);
  }
  draft_->add_fact("patch", run.patchOn ? "on" : "off");
  draft_->add_fact("textures", run.texturesOn ? "on" : "off");
  for (const Option &option : run.options.items()) {
    draft_->add_fact("option " + option.id, option.choices.at(option.value));
  }
  draft_->add_fact("frames run", std::to_string(emulator.framesRun()));
  const auto [width, height] = window_.pixelSize();
  draft_->add_fact("window", std::to_string(width) + "x" + std::to_string(height));
  draft_->add_fact("frame", std::to_string(emulator.frame().width()) + "x" +
                                std::to_string(emulator.frame().height()));
  draft_->add_fact("platform", SDL_GetPlatform());
  draft_->set_reproduction(
      {"Load the state into the same game and set",
       {"Install the set of the game the title id names, with the options the facts list.",
        "Load state.state in the tools' harness, <game> being oot3d or mm3d: loadstate takes "
        "the file's path."},
       "uv run --frozen z3dmods run <game> report --harness loadstate:<this folder>/state.state "
       "wait:1 shot:after"});
}

std::string &BugReportScreen::field() {
  return field_ == Field::Summary ? words_.summary : words_.description;
}

bool BugReportScreen::edit(const SDL_Event &event) {
  if (event.type == SDL_EVENT_TEXT_INPUT) {
    field() += event.text.text;
    return false;
  }
  if (event.type != SDL_EVENT_KEY_DOWN) {
    return false;
  }
  const bool control = (event.key.mod & SDL_KMOD_CTRL) != 0;
  switch (event.key.scancode) {
  case SDL_SCANCODE_ESCAPE:
    cancelled_ = true;
    return false;
  case SDL_SCANCODE_BACKSPACE:
    dropLast(field());
    return false;
  case SDL_SCANCODE_TAB:
    field_ = field_ == Field::Summary ? Field::Description : Field::Summary;
    return false;
  case SDL_SCANCODE_RETURN:
  case SDL_SCANCODE_KP_ENTER:
    if (control) {
      return true;
    }
    if (field_ == Field::Summary) {
      field_ = Field::Description;
    } else {
      words_.description += '\n';
    }
    return false;
  default:
    return false;
  }
}

void BugReportScreen::save() {
  if (!draft_) {
    return;
  }
  if (auto folder = draft_->commit(words_, problem_)) {
    saved_ = std::move(folder);
    problem_.clear();
    lucent::info("report", "saved {}", saved_->string());
  } else {
    lucent::info("report", "not saved: {}", problem_);
  }
}

std::optional<std::filesystem::path> BugReportScreen::run() {
  (void)window_.takeMenuRequest();
  lucent::info("report", "open{}", draft_ ? "" : " without a draft");
  SDL_StartTextInput(window_.sdlWindow());
  MenuPad before = pad_.menu();
  while (!window_.closeRequested() && !cancelled_) {
    bool asked = false;
    bool leave = false;
    (void)window_.pollMenu([&](const SDL_Event &event) {
      if (saved_) {
        leave = leave || event.type == SDL_EVENT_KEY_DOWN ||
                event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN;
        return;
      }
      asked = edit(event) || asked;
    });
    const MenuPad now = pad_.menu();
    if (!saved_ && now.b && !before.b) {
      cancelled_ = true;
    }
    if (!saved_ && now.start && !before.start) {
      asked = true;
    }
    before = now;
    if (leave || window_.takeMenuRequest()) {
      cancelled_ = !saved_;
      break;
    }
    if (asked) {
      save();
      if (saved_) {
        SDL_StopTextInput(window_.sdlWindow());
      }
    }
    draw();
  }
  // The key that closed the form would otherwise press something in the game.
  while (!window_.closeRequested() && pad_.gameInputHeld()) {
    (void)window_.pollMenu([](const SDL_Event &) {
    });
    draw();
  }
  if (!saved_) {
    lucent::info("report", "cancelled");
    if (draft_) {
      draft_->discard();
    }
  }
  return saved_;
}

float BugReportScreen::text(const std::string &line, Rgb color, SDL_FRect place) const {
  if (line.empty()) {
    return place.h;
  }
  TTF_SetFontSize(canvas_.font(), place.h);
  TTF_Text *made = canvas_.text(line, color);
  if (place.w > 0.0F) {
    TTF_SetTextWrapWidth(made, static_cast<int>(place.w));
  }
  TTF_DrawRendererText(made, place.x, place.y);
  const float tall = textSize(made).y;
  TTF_DestroyText(made);
  return tall;
}

void BugReportScreen::draw() {
  SDL_Renderer *renderer = canvas_.renderer();
  const SDL_FPoint size = canvas_.size();
  const float margin = size.x * kMargin;
  const float heading = size.y * kHeadingShare;
  const float line = size.y * kTextShare;
  canvas_.clear();
  float y = margin;
  y += text("Report a bug", kMenuText, {margin, y, 0.0F, heading}) + line;

  const float frameWidth = size.x * kFrameShare;
  const SDL_FRect frame{margin, y, frameWidth, frameWidth / frameAspect_};
  SDL_RenderTexture(renderer, frame_, nullptr, &frame);

  const float left = margin * 2.0F + frameWidth;
  const float width = size.x - left - margin;
  if (saved_) {
    float at = y;
    at += text("Saved. The game goes on when you press any key.", kMenuText,
               {left, at, width, line}) +
          line;
    text(saved_->string(), kMenuDim, {left, at, width, line});
    SDL_RenderPresent(renderer);
    return;
  }
  const auto box = [&](const char *label, const std::string &value, bool focused, float tall) {
    y += text(label, kMenuDim, {left, y, width, line}) + line * kFieldPad;
    const SDL_FRect rect{left, y, width, tall};
    canvas_.fill(focused ? kMenuSelected : kMenuTile, rect);
    const float pad = line * kFieldPad;
    canvas_.fill(kMenuTile, {rect.x + 2.0F, rect.y + 2.0F, rect.w - 4.0F, rect.h - 4.0F});
    text(value + (focused ? "_" : ""), kMenuText,
         {rect.x + pad, rect.y + pad, rect.w - pad * 2.0F, line});
    y += tall + line;
  };
  box("What went wrong? (one line)", words_.summary, field_ == Field::Summary, line * 2.0F);
  box("What you did and what you expected", words_.description, field_ == Field::Description,
      std::max(line * 6.0F, frame.y + frame.h - y - line * 2.5F));
  if (!problem_.empty()) {
    text(problem_, kMenuSelected, {left, y, width, line});
  }
  const std::array<Hint, 3> hints{
      Hint{{keyPrompt(SDL_SCANCODE_TAB)}, "Next box"},
      Hint{{keyPrompt(SDL_SCANCODE_LCTRL, &MenuPad::start), keyPrompt(SDL_SCANCODE_RETURN)},
           "Save"},
      Hint{{keyPrompt(SDL_SCANCODE_ESCAPE, &MenuPad::b)}, "Cancel"}};
  canvas_.hints(hints, pad_.lastDevice(), {size.x / 2.0F, size.y * kHintsTop}, line,
                size.x - margin * 2.0F, true);
  SDL_RenderPresent(renderer);
}

} // namespace z3d::app
