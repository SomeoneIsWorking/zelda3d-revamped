// The item screen's column of item buttons (decomp docs/pause.md): the slots replace the game's
// four buttons, so their frames go off the screen (hooks.s item_column) and so do the items on
// them, their counts and their letters, here; the column's stone stays at the widened screen's
// right edge under the slots, and the cursor stays on the grid (hooks.s item_cursor_*).
#include "../common/guest.hpp"
#include "symbols.hpp"

#include <stdint.h>

namespace z3dmods::oot3d {
namespace {

using UpdateAmmoFn = void (*)(void *layout);

// Past the widened screen's right edge, in the touch screen's pixels.
inline constexpr float kOffScreen = 1000.0F;
inline constexpr int32_t kButtonNodes[] = {kItemMenuNodeI, kItemMenuNodeX, kItemMenuNodeY,
                                           kItemMenuNodeII};

// A quad of a batch drawn past the screen's edge, through the offset its batch adds each frame.
void moveOff(uintptr_t batch, int32_t quad) {
  const auto offsets = guest::read<uintptr_t>(batch + kQuadBatchOffsets);
  *reinterpret_cast<volatile float *>(offsets + quad * kVec2fSize) = kOffScreen;
}

void moveAllOff(uintptr_t batch) {
  const auto count = guest::read<int32_t>(batch + kQuadBatchCount);
  for (int32_t quad = 0; quad < count; ++quad) {
    moveOff(batch, quad);
  }
}

} // namespace

// ItemMenu_Update's rebuild of the grid's quads (hooks.s item_grid_update). The grid's icons take
// their offsets in the rebuild; a count sets its digits' own offsets there, so they go after it.
void updateItemGrid(void *grid) {
  const auto layout = reinterpret_cast<uintptr_t>(grid);
  const auto icons = guest::read<uintptr_t>(layout + kHudItemButtonsQuads);
  for (const int32_t node : kButtonNodes) {
    moveOff(icons, node);
  }
  const auto letters = guest::read<uintptr_t>(kItemMenu + kItemMenuButtonLetters);
  moveAllOff(letters + kItemButtonLettersBatch);
  guest::function<UpdateAmmoFn>(kHudItemButtonsUpdateAmmo)(grid);
  for (const int32_t node : kButtonNodes) {
    const auto count =
        guest::read<uintptr_t>(layout + kHudItemButtonsCounts + node * sizeof(uint32_t));
    if (count != 0) {
      moveAllOff(guest::read<uintptr_t>(count + kHudCountQuads));
    }
  }
}

} // namespace z3dmods::oot3d
