// The Items and Masks screens on the top screen (decomp docs/items.md "On the top screen"): their
// draws queue the game's own touch layouts there, the grid under the title bar against the app's
// slots (hud.toml, which replace the item buttons' column, hidden with its icons and the cursor
// while it is on it) and over the item's model, the Masks
// and Items tabs down the left at full size with Return in the bottom left corner, and the item's
// description, play's message box, under the grid. The game hit-tests where it
// draws, so the stylus spreads across the top screen as at file select (top_touch_screen.cpp
// readTouch).
#include "../common/guest.hpp"
#include "symbols.hpp"
#include "top_touch_screen.hpp"
#include "ui_widget.hpp"

#include <stddef.h>
#include <stdint.h>

namespace z3dmods::mm3d {
namespace {

using LayoutDrawFn = void (*)(void *manager, void *layout, int32_t queue, int32_t order);

inline constexpr float kScreenHeight = 240.0F;
// The title bar ("Items" and the clock) the screens keep along the top screen's top.
inline constexpr float kTitleBarHeight = 30.0F;
// The left edge of the app's slots on these screens (hud.toml).
inline constexpr float kPanelLeft = 294.0F;
// The grid ends here in the touch screen's pixels, where the item buttons' column starts.
inline constexpr float kGridRight = 255.0F;
// The grid's bottom in the touch screen's pixels, above the tabs.
inline constexpr float kGridBottom = 200.0F;
inline constexpr float kPageScale = 0.75F;
inline constexpr float kPageLeft = kPanelLeft - kPageScale * kGridRight;
inline constexpr float kGridBottomOnTop = kTitleBarHeight + kPageScale * kGridBottom;
// Return's place on the touch screen, which it keeps on the top screen, and its right edge.
inline constexpr float kReturnX = 0.0F;
inline constexpr float kReturnY = 202.0F;
inline constexpr float kReturnRight = 58.0F;
// The tabs' touch areas' top left on the touch screen, Masks then Items (their bb panes), and where
// they go: one under the other left of the grid.
inline constexpr float kTabsY = 206.0F;
inline constexpr float kTabX[2] = {61.4F, 157.4F};
inline constexpr float kTabsLeft = 4.0F;
inline constexpr float kTabsTop = kTitleBarHeight + 10.0F;
inline constexpr float kTabPitch = 44.0F;
// Play's message box: its root in its middle, 80 high with its margins.
inline constexpr float kWindowHeight = 80.0F;
inline constexpr float kDescriptionScale = (kScreenHeight - kGridBottomOnTop) / kWindowHeight;
inline constexpr float kDescriptionX = (kReturnRight + kPanelLeft) / 2.0F;
inline constexpr float kDescriptionY = (kGridBottomOnTop + kScreenHeight) / 2.0F;

void queueTop(void *manager, void *layout, int32_t order) {
  guest::function<LayoutDrawFn>(kLayoutDrawManagerDraw)(manager, layout, kDrawQueueTop2, order);
}

// The roots the game places itself in the touch screen's pixels (cells, cursors, icons), put where
// the page takes them. The game's own place is kept, so a root it has not moved since is not
// placed twice.
class Roots {
public:
  void place(void *layout) {
    uint8_t *root = ui::root(layout);
    Entry &entry = find(root);
    const Place now = read(root);
    if (entry.root != root || !(now == entry.placed)) {
      entry.root = root;
      entry.game = now;
    }
    ui::place(root, kPageLeft + kPageScale * entry.game.x,
              kTitleBarHeight + kPageScale * entry.game.y, kPageScale * entry.game.scaleX,
              kPageScale * entry.game.scaleY);
    entry.placed = read(root);
    entry.used = ++clock_;
  }

private:
  struct Place {
    float x;
    float y;
    float scaleX;
    float scaleY;

    bool operator==(const Place &other) const {
      return x == other.x && y == other.y && scaleX == other.scaleX && scaleY == other.scaleY;
    }
  };
  struct Entry {
    uint8_t *root;
    Place game;
    Place placed;
    uint32_t used;
  };
  // Both screens' cells, cursors, icons and the two others ItemScreen_DrawBase queues.
  static constexpr size_t kCapacity = 64;

  static Place read(const uint8_t *root) {
    return {guest::read<float>(root, kUiWidgetTranslate),
            guest::read<float>(root, kUiWidgetTranslate + sizeof(float)),
            guest::read<float>(root, kUiWidgetScale),
            guest::read<float>(root, kUiWidgetScale + sizeof(float))};
  }

  // The root's entry, or the least recently used one for a root not seen before.
  Entry &find(const uint8_t *root) {
    Entry *oldest = &entries_[0];
    for (Entry &entry : entries_) {
      if (entry.root == root) {
        return entry;
      }
      if (entry.used < oldest->used) {
        oldest = &entry;
      }
    }
    oldest->root = nullptr;
    return *oldest;
  }

  Entry entries_[kCapacity] = {};
  uint32_t clock_ = 0;
};

// The page, with Return and the tabs kept off its scale at their own places: each one's own
// layout's root carries it there, since their translates are animated, and each tab's touch area
// (its bb pane) goes with it. Found once for each page layout.
class Page {
public:
  void place(void *layout) {
    ui::place(ui::root(layout), kPageLeft, kTitleBarHeight, kPageScale);
    if (layout != layout_) {
      layout_ = layout;
      return_ = ui::find(layout, "btn_return_l");
      tabs_[0] = ui::find(layout, "btn_omen_l");
      tabs_[1] = ui::find(layout, "btn_item_l");
      areas_[0] = ui::find(layout, "bb_item_omen_pane00");
      areas_[1] = ui::find(layout, "bb_item_omen_pane01");
      column_ = ui::find(layout, "right_item_set_l");
    }
    if (column_ != nullptr) {
      ui::hide(column_);
    }
    unscale(return_, kReturnX, kReturnY, kReturnX, kReturnY);
    for (size_t i = 0; i < 2; ++i) {
      const float toY = kTabsTop + kTabPitch * static_cast<float>(i);
      unscale(tabs_[i], kTabX[i], kTabsY, kTabsLeft, toY);
      if (areas_[i] != nullptr) {
        ui::place(areas_[i], (kTabsLeft - kPageLeft - kTabX[i]) / kPageScale,
                  (toY - kTitleBarHeight - kTabsY) / kPageScale, 1.0F / kPageScale);
      }
    }
  }

private:
  // A child with its own layout, whose point (x, y) on the touch screen is drawn at full size at
  // (toX, toY).
  static void unscale(uint8_t *child, float x, float y, float toX, float toY) {
    if (child == nullptr) {
      return;
    }
    ui::scale(child, 1.0F / kPageScale, 1.0F / kPageScale);
    const float childX = guest::read<float>(child, kUiWidgetTranslate);
    const float childY = guest::read<float>(child, kUiWidgetTranslate + sizeof(float));
    ui::place(ui::root(guest::read<void *>(child, kUiWidgetLayout)),
              toX - kPageLeft - x + (1.0F - kPageScale) * childX,
              toY - kTitleBarHeight - y + (1.0F - kPageScale) * childY, 1.0F);
  }

  void *layout_ = nullptr;
  uint8_t *return_ = nullptr;
  uint8_t *tabs_[2] = {};
  uint8_t *areas_[2] = {};
  uint8_t *column_ = nullptr;
};

// Play's message box between Return and the panel while an item screen is up, and back at the
// game's place after.
class Description {
public:
  void place(void *window, bool shown) {
    uint8_t *root = ui::root(window);
    const float x = guest::read<float>(root, kUiWidgetTranslate);
    const float y = guest::read<float>(root, kUiWidgetTranslate + sizeof(float));
    const bool placed = window == window_ && x == kDescriptionX && y == kDescriptionY;
    if (!placed) {
      window_ = window;
      gameX_ = x;
      gameY_ = y;
      gameScale_ = guest::read<float>(root, kUiWidgetScale);
    }
    if (shown) {
      ui::place(root, kDescriptionX, kDescriptionY, kDescriptionScale);
    } else if (placed) {
      ui::place(root, gameX_, gameY_, gameScale_);
    }
  }

private:
  void *window_ = nullptr;
  float gameX_ = 0.0F;
  float gameY_ = 0.0F;
  float gameScale_ = 1.0F;
};

constinit Roots roots;
constinit Page pages[2];
constinit Description description;

void drawPlaced(void *manager, void *layout, int32_t order) {
  roots.place(layout);
  queueTop(manager, layout, order);
}

} // namespace

// MessageMgr_DrawWindow's queueing of play's message box, on the top screen (hooks.s).
void drawMessageWindow(void *manager, void *window, int32_t queue, int32_t order) {
  const auto active = guest::read<uintptr_t>(kUiScreenContext + kUiActiveScreen);
  description.place(window, active == kItemsUiScreen || active == kMasksUiScreen);
  guest::function<LayoutDrawFn>(kLayoutDrawManagerDraw)(manager, window, queue, order);
}

// ItemScreen_DrawCursor's queueing of the cursor's frames, but on the item buttons' column.
void drawItemCursor(void *manager, void *layout, int32_t /*queue*/, int32_t order) {
  const auto screen = guest::read<uintptr_t>(kUiScreenContext + kUiActiveScreen);
  if ((screen == kItemsUiScreen || screen == kMasksUiScreen) &&
      guest::read<int8_t>(screen + kItemScreenCursorColumn) ==
          static_cast<int8_t>(guest::read<uint8_t>(screen + kItemScreenButtonColumn))) {
    return;
  }
  drawPlaced(manager, layout, order);
}

// ItemScreen_DrawBase's queueing of the held icons and the two others.
void drawItemBase(void *manager, void *layout, int32_t /*queue*/, int32_t order) {
  drawPlaced(manager, layout, order);
}

// ItemScreen_DrawBase's queueing of the item buttons' icons, not drawn: the app's slots replace
// them.
void skipItemButtonIcon(void * /*manager*/, void * /*layout*/, int32_t /*queue*/,
                        int32_t /*order*/) {
}

// ItemScreen_Draw's and MaskScreen_Draw's queueing of their cells.
void drawItemCell(void *manager, void *layout, int32_t /*queue*/, int32_t order) {
  drawPlaced(manager, layout, order);
}

// ItemScreen_Draw's queueing of the Items page.
void drawItemsPage(void *manager, void *layout, int32_t /*queue*/, int32_t order) {
  pages[0].place(layout);
  queueTop(manager, layout, order);
}

// MaskScreen_Draw's queueing of the Masks page.
void drawMasksPage(void *manager, void *layout, int32_t /*queue*/, int32_t order) {
  pages[1].place(layout);
  queueTop(manager, layout, order);
}

} // namespace z3dmods::mm3d
