// The Ocarina screen on the top screen (decomp docs/ocarina.md "The screen"): its draw queues the
// game's own pages there, "Play your instrument" or the song list, small in the top left corner
// clear of Link, with Cancel kept at full size in the bottom left corner. The game hit-tests where
// it draws, so the stylus spreads across the top screen (top_touch_screen.cpp readTouch).
#include "../common/guest.hpp"
#include "symbols.hpp"
#include "touch_page.hpp"
#include "ui_widget.hpp"

#include <stdint.h>

namespace z3dmods::mm3d {
namespace {

using LayoutDrawFn = void (*)(void *manager, void *layout, int32_t queue, int32_t order);

inline constexpr float kMargin = 4.0F;
inline constexpr float kScale = 0.55F;
inline constexpr PageFrame kFrame{kMargin, kMargin, kScale};

void queueTop(void *manager, void *layout, int32_t order) {
  guest::function<LayoutDrawFn>(kLayoutDrawManagerDraw)(manager, layout, kDrawQueueTop2, order);
}

// A page with Cancel's group off its scale. Found once for each page layout.
class Page {
public:
  void place(void *layout) {
    kFrame.place(layout);
    if (layout != layout_) {
      layout_ = layout;
      cancel_ = ui::find(layout, "btn_quit_g");
    }
    kFrame.keepFullSize(cancel_);
  }

private:
  void *layout_ = nullptr;
  uint8_t *cancel_ = nullptr;
};

void drawPage(Page &page, void *manager, void *layout, int32_t order) {
  page.place(layout);
  queueTop(manager, layout, order);
}

constinit Page playPage;
constinit Page listPage;
constinit FrameRoots roots;

} // namespace

// OcarinaScreen_Draw's queueing of "Play your instrument" (hooks.s).
void drawOcarinaPlay(void *manager, void *layout, int32_t /*queue*/, int32_t order) {
  drawPage(playPage, manager, layout, order);
}

// OcarinaScreen_Draw's queueing of the song list.
void drawOcarinaList(void *manager, void *layout, int32_t /*queue*/, int32_t order) {
  drawPage(listPage, manager, layout, order);
}

// OcarinaScreen_Draw's queueing of the song list's cursor.
void drawOcarinaCursor(void *manager, void *layout, int32_t /*queue*/, int32_t order) {
  roots.place(layout, kFrame);
  queueTop(manager, layout, order);
}

} // namespace z3dmods::mm3d
