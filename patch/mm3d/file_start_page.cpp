// File select's Start page as a screen of its own on the wide top screen: the file's details plate
// (the top screen's, FileSelectScreen.details) over it, and Return, Start, Copy, Erase and Options
// in a row along the bottom. FileSelectScreen_Init's build is followed by laying it out: the
// buttons and their glows, Return in the corner on every page, and the file rows' slide stretched
// off the wide screen. FileSelectScreen_UpdateStart's cursor animation puts Copy's and Erase's
// glow at their native places, so its parts are moved onto the laid-out button after each update
// (decomp docs/ui.md "Title and file select").
#include "file_start_page.hpp"

#include "../common/guest.hpp"
#include "symbols.hpp"
#include "top_touch_screen.hpp"
#include "ui_widget.hpp"

#include <stddef.h>
#include <stdint.h>

namespace z3dmods::mm3d {
namespace {

using InitFn = uint32_t (*)(void *screen, void *context);
using UpdateFn = void (*)(void *screen, void *context);
using LayoutDrawFn = void (*)(void *manager, void *layout, int32_t queue, int32_t order);

struct Rect {
  float left;
  float top;
  float right;
  float bottom;
};

struct Button {
  const char *group;
  const char *cursor; // its glow in cursor_g, or null for Copy's and Erase's shared one
  Rect native;        // its touch rect in the list's own pixels (its bb_* panes)
};

inline constexpr Button kStart{"btn_start_g", "cursor_start_g", {46, 66, 274, 110}};
inline constexpr Button kCopy{"btn_copy_g", nullptr, {66, 118, 150, 156}};
inline constexpr Button kErase{"btn_delete_g", nullptr, {170, 118, 254, 156}};
inline constexpr Button kOptions{"btn_option_g", "cursor_option_g", {94, 162, 226, 200}};
inline constexpr Button kRow[] = {kStart, kCopy, kErase, kOptions};
inline constexpr const char *kCopyEraseCursor = "cursor_copy_del_g";
inline constexpr const char *kCopyEraseCursorParts[] = {"cursor_copy_del_fx",
                                                        "cursor_copy_del_base"};

// The file rows, which the game slides 320 pixels left to the Start page, and their group: the
// slide is stretched to leave the top screen's left edge rather than the touch screen's.
inline constexpr const char *kRowsGroup = "null_plate_g";
inline constexpr const char *kRows[] = {"plate_x_n", "plate_y_n", "plate_z_n"};
inline constexpr float kRowsRest = 14.0F; // their x on the list
inline constexpr float kRowsStretch = (kTouchWidth + kTouchLeft) / kTouchWidth;
inline constexpr float kRowsTravel = kTouchWidth * kRowsStretch;

// Return's rect: the bottom left corner of the screen, the row to its right.
inline constexpr Rect kReturn{0, 202, 58, 240};
inline constexpr float kRowScale = 0.6F;
inline constexpr float kRowGap = 5.0F;
inline constexpr float kRowMiddle = (kReturn.top + kReturn.bottom) / 2.0F;
// The details plate's top in its layout, and where it goes, above the row.
inline constexpr float kPlateTop = 36.0F;
inline constexpr float kPlateMargin = 2.0F;

struct Offset {
  float x;
  float y;
};

// Each button's translate in the list, its rect scaled into the row on the top screen.
constexpr Offset rowOffset(size_t index) {
  float left = kReturn.right + kRowGap;
  for (size_t i = 0; i < index; ++i) {
    left += (kRow[i].native.right - kRow[i].native.left) * kRowScale + kRowGap;
  }
  const Rect &native = kRow[index].native;
  return {left - kTouchLeft - native.left * kRowScale,
          kRowMiddle - (native.top + native.bottom) / 2.0F * kRowScale};
}

static_assert(rowOffset(3).x + kTouchLeft + kOptions.native.right * kRowScale <= kTopWidth,
              "the row fits the top screen");

class FileStartPage {
public:
  void build(void *screen) {
    void *list = guest::read<void *>(screen, kFileSelectFiles);
    if (list == nullptr) {
      return;
    }
    firstRow_ = ui::find(list, kRows[0]);
    for (size_t i = 0; i < sizeof(kCopyEraseCursorParts) / sizeof(kCopyEraseCursorParts[0]); ++i) {
      copyEraseParts_[i] = ui::find(list, kCopyEraseCursorParts[i]);
    }
    ui::moveX(guest::read<uint8_t *>(list, kFileSelectReturnButton), kReturn.left - kTouchLeft);
    auto *rows = ui::find(list, kRowsGroup);
    ui::moveX(rows, kRowsRest - kRowsRest * kRowsStretch);
    ui::set(rows, kUiWidgetScale, kRowsStretch, kUiWidgetScaleX, 1.0F);
    for (const char *row : kRows) {
      ui::set(ui::find(list, row), kUiWidgetScale, 1.0F / kRowsStretch, kUiWidgetScaleX, 1.0F);
    }
    for (size_t i = 0; i < sizeof(kRow) / sizeof(kRow[0]); ++i) {
      const Offset at = rowOffset(i);
      ui::place(ui::find(list, kRow[i].group), at.x, at.y, kRowScale);
      if (kRow[i].cursor != nullptr) {
        ui::place(ui::find(list, kRow[i].cursor), at.x, at.y, kRowScale);
      }
    }
    // Its x is the cursor animation's; its parts carry the row's.
    ui::place(ui::find(list, kCopyEraseCursor), 0.0F, rowOffset(1).y, kRowScale);
  }

  void update(const void *screen) {
    // The animation moves the glow from Copy to Erase by their native distance, unscaled, while
    // Erase's own panes lie that distance on in the row's scale.
    const bool erase =
        guest::read<int32_t>(screen, kFileSelectStartChoice) == kFileStartChoiceErase;
    const float shift = erase ? static_cast<float>(kFileStartEraseCursorShift) : 0.0F;
    const float x = (rowOffset(erase ? 2 : 1).x - shift) / kRowScale + shift;
    for (uint8_t *part : copyEraseParts_) {
      ui::moveX(part, x);
    }
  }

  // The details plate while the Start page is up or sliding in or out: it comes in from the
  // screen's right edge as the rows leave by its left. The layout draws with the transform it had
  // before this call, so it is kept off the screen while hidden.
  void drawDetails(const void *screen, void *manager, void *layout, int32_t order) const {
    const float left = (kTouchLeft + kRowsRest - ui::drawnX(firstRow_)) / kRowsTravel;
    const float x = (1.0F - (left < 0.0F ? 0.0F : left > 1.0F ? 1.0F : left)) * kTopWidth;
    ui::place(ui::root(layout), shown(screen) ? x : kTopWidth, kPlateMargin - kPlateTop, 1.0F);
    if (!shown(screen)) {
      return;
    }
    guest::function<LayoutDrawFn>(kLayoutDrawManagerDraw)(manager, layout, kDrawQueueTop2, order);
  }

  static bool shown(const void *screen) {
    return guest::read<uint8_t>(screen, kFileSelectPage) == kFileSelectPageStart ||
           guest::read<uint8_t>(screen, kFileSelectNextPage) == kFileSelectPageStart;
  }

private:
  // The list's widgets the page reads or moves after its build: the first file row, which the
  // plate's slide follows as it is drawn every frame, and the parts of Copy's and Erase's glow.
  uint8_t *firstRow_ = nullptr;
  uint8_t *copyEraseParts_[sizeof(kCopyEraseCursorParts) / sizeof(kCopyEraseCursorParts[0])]{};
};

constinit FileStartPage startPage;

} // namespace

bool fileStartPageShown(const void *screen) {
  return FileStartPage::shown(screen);
}

// FileSelectScreen's vtable entry for FileSelectScreen_Init (hooks.s).
uint32_t initFileSelect(void *screen, void *context) {
  const uint32_t result = guest::function<InitFn>(kFileSelectScreenInit)(screen, context);
  startPage.build(screen);
  return result;
}

// FileSelectScreen_Update's call of FileSelectScreen_UpdateStart.
void updateFileStart(void *screen, void *context) {
  guest::function<UpdateFn>(kFileSelectScreenUpdateStart)(screen, context);
  startPage.update(screen);
}

// FileSelectScreen_Draw's queueing of the details plate.
void drawFileDetails(void *manager, void *layout, int32_t /*queue*/, int32_t order) {
  startPage.drawDetails(reinterpret_cast<void *>(kFileSelectUiScreen), manager, layout, order);
}

} // namespace z3dmods::mm3d
