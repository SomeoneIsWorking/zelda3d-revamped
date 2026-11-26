// The title's and file select's touch screens on the top screen, as OoT3D's file select shows its
// touch screen there: their draws queue the game's own layouts on the top screen at full size, in
// its middle. File select's list takes the place of the top screen's title bar (hooks.s
// file_title_bar) over its background, clipped to the touch screen's right edge as that edge
// clipped it, so what the game parks past it stays hidden; Return in the corner and the Start page
// (file_start_page.cpp) take the whole width. The game hit-tests touches where it draws, so the
// stylus's 320 columns span the top screen's 400 while these screens are up (decomp docs/ui.md
// "Title and file select").
#include "top_touch_screen.hpp"

#include "../common/guest.hpp"
#include "file_start_page.hpp"
#include "symbols.hpp"
#include "ui_widget.hpp"

#include <stdint.h>

namespace z3dmods::mm3d {
namespace {

using LayoutDrawFn = void (*)(void *manager, void *layout, int32_t queue, int32_t order);
using DrawQueueFn = void (*)(void *manager, int32_t queue, void *context);
using ReadTouchFn = bool (*)(void *reader, void *status);

// PRESS START, under the title's logo.
inline constexpr float kPressStartDrop = 44.0F;

struct Placement {
  float x;
  float y;
  float scale;
};

// The touch screen's frame in the middle of the top screen.
inline constexpr Placement kCentred{kTouchLeft, 0.0F, 1.0F};
inline constexpr Placement kPressStart{kCentred.x, kPressStartDrop, 1.0F};

void place(void *layout, const Placement &at) {
  ui::place(ui::root(layout), at.x, at.y, at.scale);
}

void queueLayout(void *manager, void *layout, int32_t queue, int32_t order) {
  guest::function<LayoutDrawFn>(kLayoutDrawManagerDraw)(manager, layout, queue, order);
}

// The list clipped at the touch screen's right edge as a PICA scissor in the top screen's target,
// whose x runs up the screen and y leftwards across it; the pass's own scissor, the whole target,
// is put back after.
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
      scissor(fromY << 16U, (kTopTargetWidth - 1) | (kTopTargetHeight - 1) << 16U);
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

// FileSelectScreen_Draw's queueing of the list, in a top queue of its own so it can be clipped
// while the Start page is not up.
void drawFileList(void *manager, void *layout, int32_t /*queue*/, int32_t order) {
  place(layout, kCentred);
  queueLayout(manager, layout, kDrawQueueTop1, order);
  if (!fileStartPageShown(reinterpret_cast<void *>(kFileSelectUiScreen))) {
    filesClip.arm();
  }
}

// FileSelectScreen_Draw's tail call queueing its prompts, over everything; the black that dims
// the touch screen behind them spans the top screen.
void drawFileDialogs(void *manager, void *layout, int32_t /*queue*/, int32_t order) {
  place(layout, kCentred);
  auto *black = guest::read<uint8_t *>(layout, kUiDialogBlackScreen);
  ui::place(black, -kCentred.x, 0.0F, 1.0F);
  ui::set(black, kUiWidgetScale, kTopWidth / kTouchWidth, kUiWidgetScaleX, 1.0F);
  queueLayout(manager, layout, kDrawQueueTop3, order);
}

// Input_ReadTouch's read of the stylus: across the top screen while the title, file select or the
// Items or Masks screen (item_screens.cpp) is up, as the app maps the pointer over it (overlay.toml
// touch_menus).
bool readTouch(void *reader, void *status) {
  const bool read = guest::function<ReadTouchFn>(kTouchPanelReadLatest)(reader, status);
  const auto active = guest::read<uintptr_t>(kUiScreenContext + kUiActiveScreen);
  if (active == kTitleUiScreen || active == kFileSelectUiScreen || active == kItemsUiScreen ||
      active == kMasksUiScreen || active == kGearUiScreen || active == kMapUiScreen) {
    const auto x = guest::read<uint16_t>(status, kTouchStatusX);
    guest::write(status, kTouchStatusX,
                 static_cast<uint16_t>(static_cast<float>(x) * kTopWidth / kTouchWidth + 0.5F));
  }
  return read;
}

// Graph_DrawTopScreen's draw of queue 1.
void drawTopQueue1(void *manager, int32_t queue, void *context) {
  filesClip.drawQueue(manager, queue, context);
}

} // namespace z3dmods::mm3d
