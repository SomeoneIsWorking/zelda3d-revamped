// The Map screen on the top screen (decomp docs/ui.md "Map screen"): its draw queues the touch
// page over the top screen's map at the page's own place, so the loupe the game puts at the touch
// point is drawn under the pointer, with A's button moved from beside the place name into the
// bottom right corner, X's prompt moved left of it and the panning arrows, which a drag follows
// wherever it starts, around the middle of the map. The game hit-tests where it draws, so the
// stylus spreads across the top screen (top_touch_screen.cpp readTouch).
#include "../common/guest.hpp"
#include "symbols.hpp"
#include "top_touch_screen.hpp"
#include "ui_widget.hpp"

#include <stddef.h>
#include <stdint.h>

namespace z3dmods::mm3d {
namespace {

using LayoutDrawFn = void (*)(void *manager, void *layout, int32_t queue, int32_t order);

// A's two buttons end at the touch screen's right edge; X's prompt starts at its left edge.
inline constexpr float kButtonsShift = kTopWidth - kTouchWidth;
inline constexpr float kPromptX = 300.0F;
// X's prompt, "Zoom Out" the longer, ends clear of the buttons.
inline constexpr float kPromptShift = -42.0F;

void queue(void *manager, void *layout, int32_t queue, int32_t order) {
  guest::function<LayoutDrawFn>(kLayoutDrawManagerDraw)(manager, layout, queue, order);
}

// The widgets moved, found once for each layout.
class Moved {
public:
  void page(void *layout) {
    if (layout != page_) {
      page_ = layout;
      buttons_[0] = ui::find(layout, "btn_expansion_g");
      buttons_[1] = ui::find(layout, "btn_reduction_g");
      arrows_ = ui::find(layout, "icon_touch_direct");
    }
    if (arrows_ != nullptr) {
      ui::moveX(arrows_, kTouchLeft);
    }
    for (uint8_t *button : buttons_) {
      if (button != nullptr) {
        ui::moveX(button, kButtonsShift);
      }
    }
  }

  void info(void *layout) {
    if (layout != info_) {
      info_ = layout;
      prompts_[0] = ui::find(layout, "disp_text_detail");
      prompts_[1] = ui::find(layout, "disp_text_whole");
    }
    for (uint8_t *prompt : prompts_) {
      if (prompt != nullptr) {
        ui::moveX(prompt, kPromptX + kPromptShift);
      }
    }
  }

private:
  void *page_ = nullptr;
  uint8_t *buttons_[2] = {};
  uint8_t *arrows_ = nullptr;
  void *info_ = nullptr;
  uint8_t *prompts_[2] = {};
};

constinit Moved moved;

} // namespace

// MapScreen_Draw's queueing of its touch page (hooks.s).
void drawMapPage(void *manager, void *layout, int32_t /*queue*/, int32_t order) {
  moved.page(layout);
  queue(manager, layout, kDrawQueueTop3, order);
}

// MapScreen_Draw's queueing of the top screen's place name and X's prompt.
void drawMapInfo(void *manager, void *layout, int32_t drawQueue, int32_t order) {
  moved.info(layout);
  queue(manager, layout, drawQueue, order);
}

} // namespace z3dmods::mm3d
