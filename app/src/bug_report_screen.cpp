#include "bug_report_screen.hpp"

#include "app_config.hpp"
#include "emulator.hpp"
#include "game_options.hpp"
#include "game_window.hpp"
#include "pad_input.hpp"

#include <SDL3/SDL.h>
#include <bug_report/form.h>
#include <lucent/log.h>

#include <chrono>
#include <fstream>
#include <system_error>

namespace z3d::app {
namespace {

namespace bug = lucent::bug_report;

constexpr const char *kScreenFile = "screens.png";
constexpr const char *kStateFile = "state.state";
constexpr const char *kLogFile = "z3d_core.log";

} // namespace

BugReportScreen::BugReportScreen(const MenuCanvas &canvas, GameWindow &window, const PadInput &pad,
                                 Emulator &emulator, const AppConfig &config,
                                 const ReportedRun &run)
    : window_(window), pad_(pad), host_(canvas, window, config.font) {
  capture(emulator, config.userDir, run);
}

BugReportScreen::~BugReportScreen() {
  if (state_.valid()) {
    state_.wait();
  }
  SDL_StopTextInput(window_.sdlWindow());
}

void BugReportScreen::capture(Emulator &emulator, const std::filesystem::path &userDir,
                              const ReportedRun &run) {
  const std::filesystem::path root = userDir / kFolder;
  bug::Draft::prune_abandoned(root);
  std::string error;
  draft_ = bug::Draft::open(root, "z3d", std::chrono::system_clock::now(), error);
  if (!draft_) {
    lucent::error("report", "cannot start a report: {}", error);
    return;
  }
  const auto attach = [&](const char *file, bug::AttachmentRole role, const char *label) {
    if (!draft_->attach(file, role, label, error)) {
      lucent::warn("report", "{}", error);
    }
  };
  if (const auto path = draft_->path_for(kScreenFile, error);
      path && emulator.frame().writePng(*path)) {
    attach(kScreenFile, bug::AttachmentRole::Screenshot, "Both screens as the game showed them");
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
  // The longest part of a capture; the form is up while it runs.
  if (const auto path = draft_->path_for(kStateFile, error); path && emulator.settle()) {
    state_ = std::async(std::launch::async, [&emulator, file = *path] {
      const auto state = emulator.serializeState();
      if (!state) {
        return false;
      }
      std::ofstream out(file, std::ios::binary);
      out.write(reinterpret_cast<const char *>(state->data()),
                static_cast<std::streamsize>(state->size()));
      return static_cast<bool>(out);
    });
  }
}

void BugReportScreen::attachState(bug::Draft &draft) {
  if (!state_.valid() || !state_.get()) {
    return;
  }
  std::string error;
  if (!draft.attach(kStateFile, bug::AttachmentRole::Reproduction,
                    "The emulator's state at that frame", error)) {
    lucent::warn("report", "{}", error);
  }
}

std::optional<std::filesystem::path> BugReportScreen::save(bug::Draft &draft,
                                                           bug_report::Form &form) {
  attachState(draft);
  std::string error;
  auto folder = draft.commit(form.text(), error);
  if (!folder) {
    lucent::info("report", "not saved: {}", error);
    form.rejectSave(error);
    return std::nullopt;
  }
  lucent::info("report", "saved {}", folder->string());
  return folder;
}

std::optional<std::filesystem::path> BugReportScreen::run() {
  (void)window_.takeMenuRequest();
  if (!draft_) {
    return std::nullopt;
  }
  bug::Draft &draft = *draft_;
  bug_report::Form form(host_.context(), draft, {.fontFamily = RmlHost::kFontFamily});
  lucent::info("report", "open");
  std::optional<std::filesystem::path> saved;
  MenuPad before = pad_.padMenu();
  while (!saved && !window_.closeRequested() && !window_.takeMenuRequest()) {
    (void)window_.pollMenu([&](const SDL_Event &event) {
      host_.feed(event);
    });
    const MenuPad now = pad_.padMenu();
    if (now.b && !before.b) {
      form.requestCancel();
    }
    if (now.start && !before.start) {
      form.requestSave();
    }
    before = now;
    if (form.outcome() == bug_report::Form::Outcome::Cancelled) {
      break;
    }
    if (form.outcome() == bug_report::Form::Outcome::SaveRequested) {
      saved = save(draft, form);
    }
    host_.draw();
  }
  if (!saved) {
    if (state_.valid()) {
      state_.wait();
    }
    draft.discard();
    lucent::info("report", "cancelled");
  }
  return saved;
}

} // namespace z3d::app
