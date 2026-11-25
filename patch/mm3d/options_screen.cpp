// File select's Options screen on the wide top screen: its touch list above the game's message
// box, which holds the chosen option's hint at the screen's bottom and is shared with play's text,
// so the list is scaled to fit over it, its title bar dropped (hooks.s options_title_bar), and
// Return put in the corner beside the box. OptionsScreen_Init's build is followed by laying it out:
// the list, its two pages' slide stretched off the screen's edges, Return, and the cursor's places
// on the plates (decomp docs/ui.md "Title and file select").
#include "../common/guest.hpp"
#include "symbols.hpp"
#include "top_touch_screen.hpp"
#include "ui_widget.hpp"

#include <stddef.h>
#include <stdint.h>

namespace z3dmods::mm3d {
namespace {

using InitFn = uint32_t (*)(void *screen, void *context);
using LayoutDrawFn = void (*)(void *manager, void *layout, int32_t queue, int32_t order);

inline constexpr float kTouchHeight = 240.0F;
// The message box's top and left edge on the top screen.
inline constexpr float kMessageTop = 152.0F;
inline constexpr float kMessageLeft = 50.0F;
// The list's drawn part from its first plate down, in its own pixels, scaled to end above the box.
inline constexpr float kListTop = 4.0F;
inline constexpr float kMargin = 2.0F;
inline constexpr float kScale = (kMessageTop - 2.0F * kMargin) / (kTouchHeight - kListTop);
inline constexpr float kLeft = (kTopWidth - kTouchWidth * kScale) / 2.0F;
inline constexpr float kTop = kMargin - kListTop * kScale;
// The pages slide by the touch screen's width; stretched so the one waiting leaves the screen.
// The stretches scale the pages' and tabs' parent, and their siblings back.
inline constexpr float kPageStretch = (kTopWidth - kLeft) / (kTouchWidth * kScale);
inline constexpr const char *kPagesParent = "all";
inline constexpr const char *kPagesSiblings[] = {"null_g", "btn_page2_g", "btn_page1_g", "page1_g",
                                                 "page2_g"};
// The tab rows: the game swaps them between y 0 and 40 by animation with the page, and the touch
// screen's bottom edge hid the lower one; their swap is stretched so it leaves the screen.
inline constexpr float kTabsTop = 222.0F;
inline constexpr float kTabsSwap = 40.0F;
inline constexpr float kTabsHeight = 24.0F; // with the chosen tab's rise
inline constexpr float kTabStretch =
    ((kTouchHeight - kTop) / kScale - kTabsTop + kTabsHeight) / kTabsSwap;
// Return, natively 58x38 at y 202 by animation, in the bottom left corner beside the message box;
// its own layout's root carries it down.
inline constexpr const char *kReturn = "btn_return_l";
inline constexpr float kReturnWidth = 58.0F;
inline constexpr float kReturnHeight = 38.0F;
inline constexpr float kReturnTop = 202.0F;
inline constexpr float kReturnScale = (kMessageLeft - kMargin) / kReturnWidth;

void layOut(void *list, void *cursor, float *places, uint8_t choice) {
  ui::place(ui::root(list), kLeft, kTop, kScale);
  ui::scale(ui::find(list, kPagesParent), kPageStretch, kTabStretch);
  for (const char *sibling : kPagesSiblings) {
    ui::scale(ui::find(list, sibling), 1.0F / kPageStretch, 1.0F / kTabStretch);
  }
  auto *back = ui::find(list, kReturn);
  ui::moveX(back, -kLeft / (kScale * kPageStretch));
  ui::scale(back, kReturnScale / (kScale * kPageStretch), kReturnScale / (kScale * kTabStretch));
  const float drop =
      (kTouchHeight - kReturnHeight * kReturnScale - kTop - kReturnTop * kScale * kTabStretch) /
      kReturnScale;
  ui::set(ui::root(guest::read<void *>(back, kUiWidgetLayout)), kUiWidgetTranslate + sizeof(float),
          drop, kUiWidgetTranslateY, 0.0F);
  for (size_t i = 0; i < static_cast<size_t>(kOptionsCursorPlaceCount); ++i) {
    float *place = places + i * 3;
    place[0] = kLeft + place[0] * kScale;
    place[1] = kTop + place[1] * kScale;
  }
  // The build put the cursor at the first choice's place before it was moved.
  const float *chosen = places + static_cast<size_t>(choice) * 3;
  ui::place(ui::root(cursor), chosen[0], chosen[1], kScale);
}

void queueLayout(void *manager, void *layout, int32_t queue, int32_t order) {
  guest::function<LayoutDrawFn>(kLayoutDrawManagerDraw)(manager, layout, queue, order);
}

} // namespace

// OptionsScreen's vtable entry for OptionsScreen_Init (hooks.s).
uint32_t initOptions(void *screen, void *context) {
  const uint32_t result = guest::function<InitFn>(kOptionsScreenInit)(screen, context);
  void *list = guest::read<void *>(screen, kOptionsList);
  void *cursor = guest::read<void *>(screen, kOptionsCursor);
  if (list != nullptr && cursor != nullptr) {
    layOut(list, cursor,
           reinterpret_cast<float *>(static_cast<uint8_t *>(screen) + kOptionsCursorPlaces),
           guest::read<uint8_t>(screen, kOptionsChoice));
  }
  return result;
}

// OptionsScreen_Draw's queueing of its list on the top screen.
void drawOptionsList(void *manager, void *layout, int32_t /*queue*/, int32_t order) {
  queueLayout(manager, layout, kDrawQueueTop1, order);
}

// OptionsScreen_Draw's queueing of the frame on the plate chosen, over the list.
void drawOptionsCursor(void *manager, void *layout, int32_t /*queue*/, int32_t order) {
  queueLayout(manager, layout, kDrawQueueTop3, order);
}

} // namespace z3dmods::mm3d
