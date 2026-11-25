// The title's and file select's touch screens on the top screen, as OoT3D's file select shows its
// touch screen there: their draws queue the game's own layouts on the top screen at full size, in
// its middle. File select's list takes the place of the top screen's title bar and file details
// (hooks.s file_details, file_title_bar) over its background, clipped to its 320 pixels of width as
// the touch screen's edges clipped it, so what the game parks or slides past them stays hidden. The
// game hit-tests touches where it draws (decomp docs/ui.md "Title and file select").
#include "../common/guest.hpp"
#include "symbols.hpp"

#include <stdint.h>

namespace z3dmods::mm3d {
namespace {

using LayoutDrawFn = void (*)(void *manager, void *layout, int32_t queue, int32_t order);
using DrawQueueFn = void (*)(void *manager, int32_t queue, void *context);

// The touch screen's size and the top screen's, in pixels.
inline constexpr float kTouchWidth = 320.0F;
inline constexpr float kTopWidth = 400.0F;
// PRESS START, under the title's logo.
inline constexpr float kPressStartDrop = 44.0F;

struct Placement {
  float x;
  float y;
  float scale;
};

// The touch screen's frame in the middle of the top screen.
inline constexpr Placement kCentred{(kTopWidth - kTouchWidth) / 2.0F, 0.0F, 1.0F};
inline constexpr Placement kPressStart{kCentred.x, kPressStartDrop, 1.0F};

// A root widget's field set as the game's setters do: the field's flag in flags, and in
// activeFlags whether it holds its default.
void set(uint8_t *widget, uint32_t offset, float value, uint32_t flag, float fallback) {
  if (guest::read<float>(widget, offset) == value) {
    return;
  }
  guest::write(widget, offset, value);
  guest::write(widget, kUiWidgetFlags, guest::read<uint32_t>(widget, kUiWidgetFlags) | flag);
  const auto active = guest::read<uint32_t>(widget, kUiWidgetActiveFlags) & ~flag;
  guest::write(widget, kUiWidgetActiveFlags, value == fallback ? active | flag : active);
}

void place(uint8_t *widget, float x, float y, float scaleX, float scaleY) {
  set(widget, kUiWidgetTranslate, x, kUiWidgetTranslateX, 0.0F);
  set(widget, kUiWidgetTranslate + sizeof(float), y, kUiWidgetTranslateY, 0.0F);
  set(widget, kUiWidgetScale, scaleX, kUiWidgetScaleX, 1.0F);
  set(widget, kUiWidgetScale + sizeof(float), scaleY, kUiWidgetScaleY, 1.0F);
}

void place(void *layout, const Placement &at) {
  place(static_cast<uint8_t *>(layout) + kUiLayoutRoot, at.x, at.y, at.scale, at.scale);
}

void queueLayout(void *manager, void *layout, int32_t queue, int32_t order) {
  guest::function<LayoutDrawFn>(kLayoutDrawManagerDraw)(manager, layout, queue, order);
}

// The list's width as a PICA scissor in the top screen's target, whose x runs up the screen and y
// leftwards across it; the pass's own scissor, the whole target, is put back after.
class FilesClip {
public:
  void arm() {
    armed_ = true;
  }

  void drawQueue(void *manager, int32_t queue, void *context) {
    const bool clipped = armed_;
    armed_ = false;
    if (clipped) {
      const auto fromY = static_cast<uint32_t>(kTopWidth - kCentred.x - kTouchWidth);
      const auto toY = static_cast<uint32_t>(kTopWidth - kCentred.x);
      scissor(fromY << 16U, (kTopTargetWidth - 1) | toY << 16U);
    }
    guest::function<DrawQueueFn>(kLayoutDrawManagerDrawQueue)(manager, queue, context);
    if (clipped) {
      scissor(0, (kTopTargetWidth - 1) | (kTopTargetHeight - 1) << 16U);
    }
  }

private:
  static void scissor(uint32_t from, uint32_t to) {
    write(kPicaRegScissorFrom, from);
    write(kPicaRegScissorTo, to);
    write(kPicaRegScissorMode, kPicaScissorInside);
  }

  static void write(uint32_t reg, uint32_t value) {
    auto **next = reinterpret_cast<uint32_t **>(kPicaCommandNext);
    uint32_t *at = *next;
    if (at >= guest::read<uint32_t *>(kPicaCommandEnd)) {
      return;
    }
    at[0] = value;
    at[1] = kPicaCommandAllBytes | reg;
    *next = at + 2;
  }

  // The list was queued since the queue was last drawn.
  bool armed_ = false;
};

constinit FilesClip filesClip;

} // namespace

// TitleScreen_Draw's tail call queueing PRESS START (hooks.s).
void drawTitlePressStart(void *manager, void *layout, int32_t /*queue*/, int32_t order) {
  place(layout, kPressStart);
  queueLayout(manager, layout, kDrawQueueTop3, order);
}

// FileSelectScreen_Draw's queueing of the list, in a top queue of its own so it can be clipped.
void drawFileList(void *manager, void *layout, int32_t /*queue*/, int32_t order) {
  place(layout, kCentred);
  queueLayout(manager, layout, kDrawQueueTop1, order);
  filesClip.arm();
}

// FileSelectScreen_Draw's tail call queueing its prompts, over everything; the black that dims
// the touch screen behind them spans the top screen.
void drawFileDialogs(void *manager, void *layout, int32_t /*queue*/, int32_t order) {
  place(layout, kCentred);
  place(guest::read<uint8_t *>(layout, kUiDialogBlackScreen), -kCentred.x, 0.0F,
        kTopWidth / kTouchWidth, 1.0F);
  queueLayout(manager, layout, kDrawQueueTop3, order);
}

// Graph_DrawTopScreen's draw of queue 1.
void drawTopQueue1(void *manager, int32_t queue, void *context) {
  filesClip.drawQueue(manager, queue, context);
}

} // namespace z3dmods::mm3d
