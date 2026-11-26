// The Gear screen on the top screen (decomp docs/ui.md "Gear"): its draw queues the game's own
// page there, under the title bar in the middle at the scale that leaves the bottom row its full
// size, with Return in the bottom left corner and Options under the page. The page goes on the
// queue under the title bar's, so the pictograph the game shows over the top screen stays on top.
// The game hit-tests where it draws, so the stylus spreads across the top screen
// (top_touch_screen.cpp readTouch).
#include "../common/guest.hpp"
#include "symbols.hpp"
#include "top_touch_screen.hpp"
#include "touch_page.hpp"
#include "ui_widget.hpp"

#include <stdint.h>

namespace z3dmods::mm3d {
namespace {

using LayoutDrawFn = void (*)(void *manager, void *layout, int32_t queue, int32_t order);

inline constexpr float kTitleBarHeight = 30.0F;
// The page's panels end here in touch pixels, above Return and Options.
inline constexpr float kPanelsBottom = 200.0F;
// Return's place and Options' touch area's top left, on the touch screen.
inline constexpr float kReturnX = 0.0F;
inline constexpr float kRowY = 202.0F;
inline constexpr float kOptionsX = 94.0F;
inline constexpr float kScale = (kRowY - kTitleBarHeight) / kPanelsBottom;
inline constexpr PageFrame kFrame{(kTopWidth - kScale * kTouchWidth) / 2.0F, kTitleBarHeight,
                                  kScale};

void queueTop(void *manager, void *layout, int32_t order) {
  guest::function<LayoutDrawFn>(kLayoutDrawManagerDraw)(manager, layout, kDrawQueueTop1, order);
}

// The page with Return and Options off its scale. Found once for each page layout.
class Page {
public:
  void place(void *layout) {
    kFrame.place(layout);
    if (layout != layout_) {
      layout_ = layout;
      return_ = ui::find(layout, "btn_return_l");
      options_ = ui::find(layout, "btn_option_l");
      optionsArea_ = ui::find(layout, "bb_btn_option");
    }
    kFrame.unscale(return_, kReturnX, kRowY, kReturnX, kRowY);
    kFrame.unscale(options_, kOptionsX, kRowY, kTouchLeft + kOptionsX, kRowY);
    kFrame.moveArea(optionsArea_, kOptionsX, kRowY, kTouchLeft + kOptionsX, kRowY);
  }

private:
  void *layout_ = nullptr;
  uint8_t *return_ = nullptr;
  uint8_t *options_ = nullptr;
  uint8_t *optionsArea_ = nullptr;
};

constinit Page page;
constinit FrameRoots roots;

} // namespace

// GearScreen_Draw's queueing of its page (hooks.s).
void drawGearPage(void *manager, void *layout, int32_t /*queue*/, int32_t order) {
  page.place(layout);
  queueTop(manager, layout, order);
}

// GearScreen_Draw's queueing of its cursor.
void drawGearCursor(void *manager, void *layout, int32_t /*queue*/, int32_t order) {
  roots.place(layout, kFrame);
  queueTop(manager, layout, order);
}

} // namespace z3dmods::mm3d
