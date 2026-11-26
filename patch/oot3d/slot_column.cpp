// The item slots' column, drawn by the game where a screen assigns them: the item screen and the
// ocarina's song list. Its layouts are of the game's own kind, built after the item screen's: the
// item screen's column of stone at the top screen's right edge and, for each slot, the play HUD's
// ring with the slot's item, its count and its label, at hud.toml's place for the slot on the
// device in use (slot_places.hpp). The ring and the labels are cells of the message text's prompt
// sheet, loaded again here, which the texture pack draws (sets/hud) and whose keyboard image it
// swaps in while the keyboard is in use. On the item screen the column replaces the game's own and
// its four buttons, whose quads go off the screen (hooks.s item_column) with the items on them,
// their counts and their letters; the cursor stays on the grid (hooks.s item_cursor_*). Decomp
// docs/pause.md.
#include "../common/guest.hpp"
#include "game.hpp"
#include "inventory_screen.hpp"
#include "menu_layout.hpp"
#include "slot_link.hpp"
#include "slot_places.hpp"
#include "symbols.hpp"

#include <stdint.h>

namespace z3dmods::oot3d {
namespace {

using SetIconFn = void (*)(void *layout, int32_t node, int32_t item);
using LayoutFn = void (*)(void *layout);
using TextureGetFn = void *(*)(int32_t slot);
using ResetFn = void (*)();
using MbstowcsFn = uint32_t (*)(uint16_t *wide, const char *text, uint32_t length);
using FileRequestNewFn = void *(*)(const uint16_t *path, int32_t, int32_t, uint8_t);
using FileRequestWaitFn = bool (*)(void *request, int32_t block);
using FileRequestDataFn = void *(*)(void *request);
using TextureInitFn = void *(*)(void *texture, void *data, int32_t, int32_t);
using FreeFn = void (*)(void *memory);
using AllocFn = void *(*)(void *allocator, uint32_t size);
// The game passes floats in VFP registers; the patch is built softfp.
// NOLINTNEXTLINE(modernize-use-using): only a typedef takes the calling convention.
typedef void (*SetRectsFn)(void *batch, const float *corner, const float *size, int32_t count,
                           int32_t first, float z) __attribute__((pcs("aapcs-vfp")));

// Past the top screen's right edge.
inline constexpr float kOffScreen = 1000.0F;
inline constexpr int32_t kButtonNodes[] = {kItemMenuNodeI, kItemMenuNodeX, kItemMenuNodeY,
                                           kItemMenuNodeII};
// The column's cell is sampled this far inside its left edge, in its sheet's pixels: the texture
// pack's copy carries a neighbouring sprite's edge in the cell's first texels, which drew as a
// light line down the column.
inline constexpr float kColumnInset = 0.5F;
// A slot's icon in its ring, as the play HUD draws it: this percent of the ring, centred.
inline constexpr int32_t kIconPercent = 68;
// A count's digit, the game's 11 by 12 cell, drawn half an icon tall at the icon's bottom right
// and hanging a third of a digit below it.
inline constexpr float kDigitAspect = 11.0F / 12.0F;
inline constexpr float kDigitShare = 0.5F;
inline constexpr float kDigitHang = 1.0F / 3.0F;
// A quad's first corner's alpha in QuadBatch.colors: RGBA.
inline constexpr uint32_t kAlpha = 3 * sizeof(float);
// A quad's colours in QuadBatch.colors: four corners' RGBA.
inline constexpr uint32_t kQuadColorBytes = 16 * sizeof(float);
// A slot's label, as wide as this percent of its ring, hangs a quarter of its side off the
// ring's top left corner.
inline constexpr int32_t kLabelPercent = 46;
inline constexpr uint32_t kLoneLabels = sizeof(kSlotLabels) / sizeof(SlotLabel);
inline constexpr uint32_t kLabelQuads = kSlotCount + kLoneLabels;
// gGfxAllocator's vtable entry that allocates.
inline constexpr uint32_t kAllocEntry = 2 * sizeof(uint32_t);

// `percent` of `size`, rounded.
int32_t share(int32_t size, int32_t percent) {
  return (size * percent + 50) / 100;
}

// A texture loaded from the romfs as MenuTextures_Load loads the menu's.
void *loadTexture(const char *path) {
  uint16_t wide[kMenuTexturePathLength];
  guest::function<MbstowcsFn>(kMbstowcs)(wide, path, kMenuTexturePathLength);
  void *request = guest::function<FileRequestNewFn>(kFileRequestNew)(wide, 0, 0, 0);
  guest::function<FileRequestWaitFn>(kFileRequestWait)(request, 1);
  const auto allocator = guest::read<uintptr_t>(kGfxAllocator);
  const auto alloc = guest::read<AllocFn>(guest::read<uintptr_t>(allocator) + kAllocEntry);
  void *data = guest::function<FileRequestDataFn>(kFileRequestData)(request);
  void *texture = guest::function<TextureInitFn>(kTextureInit)(
      alloc(reinterpret_cast<void *>(allocator), kTextureSize), data, 0, 0);
  guest::function<FreeFn>(kSystemArenaFree)(data);
  guest::function<FreeFn>(kFileRequestDelete)(request);
  return texture;
}

uintptr_t quadsOf(uintptr_t layout) {
  return guest::read<uintptr_t>(layout + kHudItemButtonsQuads);
}

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

class SlotColumn {
public:
  // After ItemMenu_Init has built the item screen's layouts.
  void build() {
    column_.create(1);
    rings_.create(kSlotCount);
    items_.create(kSlotCount);
    labels_.create(kLabelQuads);
    // The column is the item screen's cell of it, drawn at the cell's size.
    const auto screen = guest::read<uintptr_t>(kItemMenu + kItemMenuQuads);
    const auto *screenSize =
        reinterpret_cast<const volatile float *>(screen + kQuadBatchTextureSize);
    column_.sheet(guest::function<TextureGetFn>(kMenuTextureGet)(kMenuTextureItemParts),
                  screenSize[0], screenSize[1]);
    const auto *cells = reinterpret_cast<const float *>(kItemMenuQuadUvs);
    const auto *sizes = reinterpret_cast<const float *>(kItemMenuQuadUvSizes);
    column_.cell(
        0, {cells[2 * kItemMenuQuadColumn] + kColumnInset, cells[2 * kItemMenuQuadColumn + 1]},
        {sizes[2 * kItemMenuQuadColumn] - kColumnInset, sizes[2 * kItemMenuQuadColumn + 1]});
    columnWidth_ = static_cast<int32_t>(sizes[2 * kItemMenuQuadColumn]);
    columnHeight_ = static_cast<int32_t>(sizes[2 * kItemMenuQuadColumn + 1]);
    // Each ring is the ring's cell; the labels are slot n's cell, then each lone label's.
    void *slotSheet = loadTexture(kSlotSheet);
    rings_.sheet(slotSheet, kSlotSheetSize[0], kSlotSheetSize[1]);
    labels_.sheet(slotSheet, kSlotSheetSize[0], kSlotSheetSize[1]);
    const float cell[2] = {kSlotSheetCell, kSlotSheetCell};
    for (uint32_t slot = 0; slot < kSlotCount; ++slot) {
      rings_.cell(slot, {kSlotRingCell[0], kSlotRingCell[1]}, cell);
      labels_.cell(slot,
                   {static_cast<float>(kSlotLabelRow[0] + slot * kSlotSheetCell),
                    static_cast<float>(kSlotLabelRow[1])},
                   cell);
    }
    for (uint32_t lone = 0; lone < kLoneLabels; ++lone) {
      labels_.cell(
          kSlotCount + lone,
          {static_cast<float>(kSlotLabels[lone].u), static_cast<float>(kSlotLabels[lone].v)}, cell);
    }
  }

  // Each frame of a screen it is on: every slot at its place with what it holds, at `alpha`;
  // `left` is the top screen's left edge in that screen's pixels.
  void update(float alpha, float left) {
    const volatile SlotLink &link = slotLink();
    const auto device = static_cast<HostDevice>(link.device);
    const auto offset = static_cast<int32_t>(left);
    column_.place(0, static_cast<int32_t>(left + kTopWidth) - columnWidth_, 0, columnWidth_,
                  columnHeight_);
    for (uint32_t node = 0; node < kSlotCount; ++node) {
      // A slot the device has no place for is drawn past the edge.
      const SlotPlace *place = nullptr;
      for (const SlotPlace &each : kSlotPlaces) {
        if (each.device == device && each.slot == node) {
          place = &each;
        }
      }
      const int32_t size = place != nullptr ? place->size : 0;
      const int32_t x = place != nullptr ? place->x + offset : static_cast<int32_t>(kOffScreen);
      const int32_t y = place != nullptr ? place->y : 0;
      rings_.place(node, x, y, size, size);
      const int32_t icon = share(size, kIconPercent);
      const int32_t inset = (size - icon) / 2;
      const uint8_t song = link.songs[node];
      guest::function<SetIconFn>(kHudItemButtonsSetIcon)(
          reinterpret_cast<void *>(items_.address()), static_cast<int32_t>(node),
          song != 0 ? kItemSongMinuet + song - 1 : link.items[node]);
      items_.place(node, x + inset, y + inset, icon, icon);
      placeCount(node, static_cast<float>(x + inset + icon), static_cast<float>(y + inset + icon),
                 static_cast<float>(icon));
      const bool labelled = place != nullptr && place->labelled;
      const int32_t label = share(size, kLabelPercent);
      labels_.place(node, labelled ? x - label / 4 : static_cast<int32_t>(kOffScreen),
                    y - label / 4, label, label);
    }
    for (uint32_t lone = 0; lone < kLoneLabels; ++lone) {
      const SlotLabel &label = kSlotLabels[lone];
      labels_.place(kSlotCount + lone,
                    label.device == device ? label.x + offset : static_cast<int32_t>(kOffScreen),
                    label.y, label.size, label.size);
    }
    column_.fade(alpha);
    rings_.fade(alpha);
    items_.fade(alpha);
    labels_.fade(alpha);
  }

  // The top screen's width left of the column.
  [[nodiscard]] float spaceLeft() const {
    return kTopWidth - static_cast<float>(columnWidth_);
  }

  // Bottom to top: the column, the rings, the items and their counts, the labels.
  void draw() const {
    column_.draw();
    rings_.draw();
    items_.draw();
    labels_.draw();
  }

private:
  // HudItemButtons_SetRect2D sizes a count for a 42 pixel icon; a slot's goes at its icon's
  // corner, the ones rightmost, with `right` and `bottom` the icon's edges.
  void placeCount(uint32_t node, float right, float bottom, float icon) const {
    const auto count =
        guest::read<uintptr_t>(items_.address() + kHudItemButtonsCounts + node * sizeof(uint32_t));
    if (count == 0) {
      return;
    }
    const float height = icon * kDigitShare;
    const float size[2] = {height * kDigitAspect, height};
    const auto digits = guest::read<int32_t>(count + kHudCountDigits);
    for (int32_t digit = 0; digit < digits; ++digit) {
      const float corner[2] = {right - static_cast<float>(digit + 1) * size[0],
                               bottom - height + height * kDigitHang};
      guest::function<SetRectsFn>(kQuadBatchSetRects)(guest::read<void *>(count + kHudCountQuads),
                                                      corner, size, 1, digit, 0.0F);
    }
  }

  MenuLayout column_;
  MenuLayout rings_;
  MenuLayout items_;
  MenuLayout labels_;
  int32_t columnWidth_ = 0;
  int32_t columnHeight_ = 0;
};

constinit SlotColumn slotColumn;

} // namespace

// ItemMenu_Init's last call (hooks.s item_menu_init): the screen is built.
void resetItemMenu() {
  guest::function<ResetFn>(kItemMenuReset)();
  buildInventoryScreen();
  slotColumn.build();
}

// ItemMenu_Update's rebuild of the grid's quads (hooks.s item_grid_update). The grid's icons take
// their offsets in the rebuild; a count sets its digits' own offsets there, so they go after it.
void updateItemGrid(void *grid) {
  const auto layout = reinterpret_cast<uintptr_t>(grid);
  for (const int32_t node : kButtonNodes) {
    moveOff(quadsOf(layout), node);
  }
  const auto letters = guest::read<uintptr_t>(kItemMenu + kItemMenuButtonLetters);
  moveAllOff(letters + kItemButtonLettersBatch);
  guest::function<LayoutFn>(kHudItemButtonsUpdateAmmo)(grid);
  for (const int32_t node : kButtonNodes) {
    const auto count =
        guest::read<uintptr_t>(layout + kHudItemButtonsCounts + node * sizeof(uint32_t));
    if (count != 0) {
      moveAllOff(guest::read<uintptr_t>(count + kHudCountQuads));
    }
  }
  // The inventory and the column fade with the screen: its Gear tab's alpha, a quad always drawn.
  const auto screen = guest::read<uintptr_t>(kItemMenu + kItemMenuQuads);
  const auto alpha = guest::read<float>(guest::read<uintptr_t>(screen + kQuadBatchColors) +
                                        kItemMenuQuadTabs * kQuadColorBytes + kAlpha);
  updateInventoryScreen(alpha);
  slotColumn.update(alpha, -kWideMargin);
}

// ItemMenu_Draw's draw of the layout at +0x40 (hooks.s item_menu_draw): the column goes under it.
void drawItemColumn(void *layout) {
  slotColumn.draw();
  guest::function<LayoutFn>(kHudItemButtonsDraw)(layout);
}

// The One Screen's draw of the ocarina's screen on the top screen, from its HUD draw (hooks.s
// base_ocarina_draw): on the song list the column goes over it, in the top screen's pixels.
void drawTopOcarina() {
  guest::function<ResetFn>(kBaseOcarinaDraw)();
  if (guest::read<int32_t>(kOcarinaScreenState) == kOcarinaScreenSongList) {
    slotColumn.update(1.0F, 0.0F);
    slotColumn.draw();
  }
}

// The One Screen's song list transform (hooks.s base_song_list_width): the list centres in the
// space left of the column rather than on the whole top screen.
float songListWidth() {
  return slotColumn.spaceLeft();
}

} // namespace z3dmods::oot3d
