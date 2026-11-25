#include "options_menu.hpp"

#include "game_options.hpp"
#include "game_window.hpp"
#include "pad_input.hpp"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <lucent/log.h>

#include <algorithm>
#include <array>
#include <string_view>
#include <utility>

namespace z3d::app {
namespace {

constexpr const char *kSelect = "Select";
constexpr const char *kOpen = "Open";
constexpr const char *kChange = "Change";
constexpr const char *kBack = "Back";
constexpr const char *kFolders = "Games and mods folders";
constexpr const char *kFoldersChange = "Change";
constexpr const char *kFoldersRow = "folders";
constexpr const char *kFullscreen = "Fullscreen";
constexpr const char *kFullscreenRow = "fullscreen";
constexpr const char *kDisplay = "Display";
constexpr const char *kSystem = "System";
// The categories in the order they are listed; one not named here comes after them.
constexpr std::array<std::string_view, 5> kCategoryOrder{"Gameplay", "Controls", kDisplay, "Sound",
                                                         kSystem};
constexpr SDL_Color kChosen{224, 186, 72, SDL_ALPHA_OPAQUE};
constexpr SDL_Color kPlain{236, 238, 242, SDL_ALPHA_OPAQUE};
constexpr SDL_Color kDim{140, 144, 152, SDL_ALPHA_OPAQUE};
// Text heights as shares of the window's height, capped by its width so a row fits a narrow window.
constexpr float kHeadingShare = 1.0F / 16.0F;
constexpr float kRowShare = 1.0F / 24.0F;
constexpr float kHeadingWidthShare = 1.0F / 16.0F;
constexpr float kRowWidthShare = 1.0F / 27.0F;
constexpr float kMargin = 0.06F;
// The rows' left edge, right of the categories.
constexpr float kRowsLeft = 0.3F;
constexpr float kRowPitch = 1.9F;
constexpr float kRowsTop = 0.24F;
constexpr float kHintsTop = 0.9F;

struct Held {
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
  bool change = false;
  bool back = false;

  static Held from(const MenuPad &pad) {
    return {pad.up, pad.down, pad.left, pad.right, pad.a, pad.b || pad.x};
  }
};

} // namespace

OptionsMenu::OptionsMenu(const MenuCanvas &canvas, GameWindow &window, const PadInput &pad,
                         GameOptions &options, std::string title, bool folders)
    : canvas_(canvas), renderer_(canvas.renderer()), engine_(canvas.engine()),
      font_(canvas.font()), window_(window), pad_(pad), options_(options),
      title_(std::move(title)) {
  const auto add = [this](std::string_view name, Row row) {
    auto found = std::ranges::find(categories_, name, &Category::name);
    if (found == categories_.end()) {
      categories_.push_back({std::string(name), {}});
      found = categories_.end() - 1;
    }
    found->rows.push_back(row);
  };
  for (size_t i = 0; i < options_.items().size(); ++i) {
    add(options_.items()[i].category, {Row::Kind::Option, i});
  }
  add(kDisplay, {Row::Kind::Fullscreen, 0});
  if (folders) {
    add(kSystem, {Row::Kind::Folders, 0});
  }
  std::ranges::stable_sort(categories_, {}, [](const Category &category) {
    return std::ranges::find(kCategoryOrder, category.name) - kCategoryOrder.begin();
  });
}

std::string OptionsMenu::rowId(const Row &row) const {
  switch (row.kind) {
  case Row::Kind::Fullscreen:
    return kFullscreenRow;
  case Row::Kind::Folders:
    return kFoldersRow;
  case Row::Kind::Option:
    break;
  }
  return options_.items()[row.option].id;
}

void OptionsMenu::pickCategory(size_t index) {
  category_ = index;
  selected_ = 0;
  first_ = 0;
  std::string rows;
  for (const Row &each : categories_[category_].rows) {
    rows += " " + rowId(each);
  }
  lucent::info("menu", "options: category {}:{}", categories_[category_].name, rows);
}

void OptionsMenu::select(size_t index) {
  selected_ = index;
  lucent::info("menu", "options: row {}", rowId(row()));
}

void OptionsMenu::step(int direction) {
  switch (row().kind) {
  case Row::Kind::Folders:
    return;
  case Row::Kind::Fullscreen:
    window_.setFullscreen(!window_.fullscreen());
    return;
  case Row::Kind::Option:
    break;
  }
  options_.step(row().option, direction);
  const Option &item = options_.items()[row().option];
  lucent::info("menu", "options: {} = {}", item.id, item.choices[item.value]);
}

OptionsMenu::Left OptionsMenu::run() {
  (void)window_.takeMenuRequest();
  (void)window_.takeSettingsRequest();
  Held before = Held::from(pad_.menu());
  lucent::info("menu", "options: open for {}", title_);
  pickCategory(category_);
  if (inRows_) {
    select(selected_);
  }
  while (!window_.closeRequested()) {
    (void)window_.pollMenu();
    const Held now = Held::from(pad_.menu());
    const auto pressed = [&](bool Held::*button) {
      return now.*button && !(before.*button);
    };
    if (window_.takeMenuRequest() || window_.takeSettingsRequest()) {
      break;
    }
    if (!inRows_) {
      const size_t count = categories_.size();
      if (pressed(&Held::up)) {
        pickCategory((category_ + count - 1) % count);
      }
      if (pressed(&Held::down)) {
        pickCategory((category_ + 1) % count);
      }
      if (pressed(&Held::right) || pressed(&Held::change)) {
        inRows_ = true;
        select(0);
      } else if (pressed(&Held::back)) {
        break;
      }
    } else {
      const size_t count = categories_[category_].rows.size();
      if (pressed(&Held::up)) {
        select((selected_ + count - 1) % count);
      }
      if (pressed(&Held::down)) {
        select((selected_ + 1) % count);
      }
      if (pressed(&Held::change) && row().kind == Row::Kind::Folders) {
        lucent::info("menu", "options: closed for the folders");
        return Left::ForFolders;
      }
      if (pressed(&Held::left)) {
        step(-1);
      }
      if (pressed(&Held::right) || pressed(&Held::change)) {
        step(1);
      }
      if (pressed(&Held::back)) {
        inRows_ = false;
        pickCategory(category_);
      }
    }
    before = now;
    draw();
  }
  lucent::info("menu", "options: closed");
  return Left::Back;
}

void OptionsMenu::text(const std::string &line, float x, float y, SDL_Color color,
                       bool rightAligned) {
  TTF_Text *made = TTF_CreateText(engine_, font_, line.c_str(), line.size());
  if (made == nullptr) {
    return;
  }
  TTF_SetTextColor(made, color.r, color.g, color.b, color.a);
  int w = 0;
  int h = 0;
  TTF_GetTextSize(made, &w, &h);
  TTF_DrawRendererText(made, rightAligned ? x - static_cast<float>(w) : x, y);
  TTF_DestroyText(made);
}

void OptionsMenu::draw() {
  int w = 0;
  int h = 0;
  SDL_GetCurrentRenderOutputSize(renderer_, &w, &h);
  const auto width = static_cast<float>(std::max(w, 1));
  const auto height = static_cast<float>(std::max(h, 1));
  SDL_SetRenderDrawColor(renderer_, 18, 20, 26, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer_);
  const float left = width * kMargin;
  const float rowsLeft = width * kRowsLeft;
  const float right = width * (1.0F - kMargin);
  TTF_SetFontSize(font_, std::min(height * kHeadingShare, width * kHeadingWidthShare));
  text(title_, left, height * 0.08F, kPlain, false);
  const float size = std::min(height * kRowShare, width * kRowWidthShare);
  TTF_SetFontSize(font_, size);
  const float pitch = size * kRowPitch;
  float y = height * kRowsTop;
  for (size_t i = 0; i < categories_.size(); ++i) {
    const bool chosen = i == category_;
    text(categories_[i].name, left, y, chosen && !inRows_ ? kChosen : chosen ? kPlain : kDim, false);
    y += pitch;
  }
  // The rows that fit above the hints; the list scrolls to keep the selected one among them.
  const std::vector<Row> &rows = categories_[category_].rows;
  const auto shown =
      std::max<size_t>(static_cast<size_t>(height * (kHintsTop - kRowsTop) / pitch), 1);
  first_ = std::clamp(first_, selected_ + 1 > shown ? selected_ + 1 - shown : 0, selected_);
  const size_t last = std::min(rows.size(), first_ + shown);
  y = height * kRowsTop;
  for (size_t i = first_; i < last; ++i) {
    const SDL_Color color = inRows_ && i == selected_ ? kChosen : kPlain;
    switch (rows[i].kind) {
    case Row::Kind::Option: {
      const Option &item = options_.items()[rows[i].option];
      text(item.label, rowsLeft, y, color, false);
      text(item.choices[item.value], right, y, color, true);
      break;
    }
    case Row::Kind::Fullscreen:
      text(kFullscreen, rowsLeft, y, color, false);
      text(window_.fullscreen() ? "On" : "Off", right, y, color, true);
      break;
    case Row::Kind::Folders:
      text(kFolders, rowsLeft, y, color, false);
      text(kFoldersChange, right, y, color, true);
      break;
    }
    y += pitch;
  }
  // Back is Esc on the keyboard, the key that opens the options over a game.
  Prompt back = PadInput::menuPrompt(&MenuPad::b);
  back.key = kSettingsKey;
  const Hint select{{PadInput::menuPrompt(&MenuPad::up), PadInput::menuPrompt(&MenuPad::down)},
                    kSelect};
  const std::array hints{
      select, Hint{{PadInput::menuPrompt(&MenuPad::a)}, inRows_ ? kChange : kOpen},
      Hint{{back}, kBack}};
  canvas_.hints(hints, pad_.lastDevice(), {left, height * kHintsTop}, size, right - left, false);
  SDL_RenderPresent(renderer_);
}

} // namespace z3d::app
