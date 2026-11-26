// The item slots' column, drawn by the game where a screen assigns them: the item screen and the
// ocarina's song list. Its layouts are of the game's own kind, built after the item screen's: the
// item screen's column of stone at the top screen's right edge and, for each slot, one of its
// round buttons, its shadow and stone from that screen's sheet, with the slot's item, its count and
// its label, at hud.toml's place for the slot on the device in use (slot_places.hpp). The labels
// are cells of the message text's prompt sheet, loaded again here, whose keyboard image the texture
// pack swaps in while the keyboard is in use. On the item screen the
// column replaces the game's own and its four buttons, whose quads go off the screen (hooks.s
// item_column) with the items on them, their counts and their letters; the cursor stays on the
// grid (hooks.s item_cursor_*). Decomp docs/pause.md.
#include "../common/guest.hpp"
#include "game.hpp"
#include "slot_link.hpp"
#include "slot_places.hpp"
#include "symbols.hpp"

#include <stdint.h>

namespace z3dmods::oot3d {
namespace {

using NewFn = void *(*)(uint32_t size);
using InitFn = void *(*)(void *layout, int32_t count, int32_t kind, int32_t noCounts);
using LayoutFn = void (*)(void *layout);
using SetIconFn = void (*)(void *layout, int32_t node, int32_t item);
using SetRectFn = void (*)(void *layout, int32_t node, int32_t x, int32_t y, int32_t width,
                           int32_t height);
using SetUvsFn = void (*)(void *batch, const float *corner, const float *size, int32_t count,
                          int32_t first);
using SetRectsFn = void (*)(void *batch, const float *corner, const float *size, int32_t count,
                            int32_t first);
using TextureGetFn = void *(*)(int32_t slot);
using SetTextureFn = void (*)(void *model, int32_t unit, void *texture, uint16_t minFilter,
                              uint16_t magFilter, uint16_t wrapS, uint16_t wrapT);
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
typedef void (*SetAlphaFn)(void *layout, int32_t node, float alpha)
    __attribute__((pcs("aapcs-vfp")));

// Past the top screen's right edge.
inline constexpr float kOffScreen = 1000.0F;
// The column's stone, the first quad of the stones' layout; the slots' buttons follow it.
inline constexpr int32_t kColumnQuads = 1;
inline constexpr uint32_t kStoneQuads = kColumnQuads + kSlotCount * kItemMenuRoundButtonQuads;
inline constexpr int32_t kButtonNodes[] = {kItemMenuNodeI, kItemMenuNodeX, kItemMenuNodeY,
                                           kItemMenuNodeII};
// HudItemButtons_Init's kind for a layout of items.
inline constexpr int32_t kItemsKind = 0;
// GL_LINEAR and GL_CLAMP_TO_EDGE, as ItemMenu_Init binds the screen's sheet.
inline constexpr uint16_t kLinear = 0x2601;
inline constexpr uint16_t kClampToEdge = 0x812F;
// A round button of the column, in the screen's pixels: its shadow's halves fill a 48 pixel
// square and its stone is 44 pixels at (2, 2) (gItemMenuQuadPositions).
inline constexpr int32_t kRoundButton = 48;
inline constexpr int32_t kRoundStone = 44;
inline constexpr int32_t kRoundStoneInset = 2;
// A count's digit, the game's 11 by 12 cell, drawn half a stone tall at the stone's bottom right
// and hanging a third of a digit below it.
inline constexpr float kDigitAspect = 11.0F / 12.0F;
inline constexpr float kDigitShare = 0.5F;
inline constexpr float kDigitHang = 1.0F / 3.0F;
// A quad's first corner's alpha in QuadBatch.colors: RGBA.
inline constexpr uint32_t kAlpha = 3 * sizeof(float);
// A slot's label, as wide as this percent of its button, hangs a quarter of its side off the
// button's top left corner.
inline constexpr int32_t kLabelPercent = 46;
inline constexpr uint32_t kLoneLabels = sizeof(kSlotLabels) / sizeof(SlotLabel);
inline constexpr uint32_t kLabelQuads = kSlotCount + kLoneLabels;
// gGfxAllocator's vtable entry that allocates.
inline constexpr uint32_t kAllocEntry = 2 * sizeof(uint32_t);

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
    stones_ = layout(kStoneQuads);
    items_ = layout(kSlotCount);
    labels_ = layout(kLabelQuads);
    // The stones take the item screen's sheet, its column's cell and its round button's.
    const auto screen = guest::read<uintptr_t>(kItemMenu + kItemMenuQuads);
    const uintptr_t batch = quadsOf(stones_);
    const auto *screenSize =
        reinterpret_cast<const volatile float *>(screen + kQuadBatchTextureSize);
    sheet(stones_, guest::function<TextureGetFn>(kMenuTextureGet)(kMenuTextureItemParts),
          screenSize[0], screenSize[1]);
    const auto *cells = reinterpret_cast<const float *>(kItemMenuQuadUvs);
    const auto *sizes = reinterpret_cast<const float *>(kItemMenuQuadUvSizes);
    const auto setUvs = guest::function<SetUvsFn>(kQuadBatchSetUvs);
    setUvs(reinterpret_cast<void *>(batch), cells + 2 * kItemMenuQuadColumn,
           sizes + 2 * kItemMenuQuadColumn, kColumnQuads, 0);
    for (uint32_t slot = 0; slot < kSlotCount; ++slot) {
      setUvs(reinterpret_cast<void *>(batch), cells + 2 * kItemMenuQuadRoundButton,
             sizes + 2 * kItemMenuQuadRoundButton, kItemMenuRoundButtonQuads,
             static_cast<int32_t>(kColumnQuads + slot * kItemMenuRoundButtonQuads));
    }
    // The column is drawn at its cell's size, against the widened screen's right edge.
    columnWidth_ = static_cast<int32_t>(sizes[2 * kItemMenuQuadColumn]);
    columnHeight_ = static_cast<int32_t>(sizes[2 * kItemMenuQuadColumn + 1]);
    // The labels: slot n's cell, then each lone label's.
    sheet(labels_, loadTexture(kSlotLabelSheet), kSlotLabelSheetSize[0], kSlotLabelSheetSize[1]);
    const float cell[2] = {kSlotLabelCell, kSlotLabelCell};
    for (uint32_t slot = 0; slot < kSlotCount; ++slot) {
      const float at[2] = {static_cast<float>(kSlotLabelRow[0] + slot * kSlotLabelCell),
                           static_cast<float>(kSlotLabelRow[1])};
      setUvs(reinterpret_cast<void *>(quadsOf(labels_)), at, cell, 1, static_cast<int32_t>(slot));
    }
    for (uint32_t lone = 0; lone < kLoneLabels; ++lone) {
      const float at[2] = {static_cast<float>(kSlotLabels[lone].u),
                           static_cast<float>(kSlotLabels[lone].v)};
      setUvs(reinterpret_cast<void *>(quadsOf(labels_)), at, cell, 1,
             static_cast<int32_t>(kSlotCount + lone));
    }
  }

  // Each frame of a screen it is on: every slot at its place with what it holds, at `alpha`;
  // `left` is the top screen's left edge in that screen's pixels.
  void update(float alpha, float left) {
    const volatile SlotLink &link = slotLink();
    const auto device = static_cast<HostDevice>(link.device);
    const auto setRect = guest::function<SetRectFn>(kHudItemButtonsSetRect);
    auto *stones = reinterpret_cast<void *>(stones_);
    auto *items = reinterpret_cast<void *>(items_);
    auto *labels = reinterpret_cast<void *>(labels_);
    const auto offset = static_cast<int32_t>(left);
    setRect(stones, 0, static_cast<int32_t>(left + kTopWidth) - columnWidth_, 0, columnWidth_,
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
      const int32_t half = size / 2;
      const int32_t inset = kRoundStoneInset * size / kRoundButton;
      const int32_t stone = kRoundStone * size / kRoundButton;
      const auto first = static_cast<int32_t>(kColumnQuads + node * kItemMenuRoundButtonQuads);
      setRect(stones, first, x, y, half, size);
      setRect(stones, first + 1, x + half, y, size - half, size);
      setRect(stones, first + 2, x + inset, y + inset, stone, stone);
      const uint8_t song = link.songs[node];
      guest::function<SetIconFn>(kHudItemButtonsSetIcon)(items, static_cast<int32_t>(node),
                                                         song != 0 ? kItemSongMinuet + song - 1
                                                                   : link.items[node]);
      setRect(items, static_cast<int32_t>(node), x + inset, y + inset, stone, stone);
      placeCount(node, static_cast<float>(x + inset + stone), static_cast<float>(y + inset + stone),
                 static_cast<float>(stone));
      const bool labelled = place != nullptr && place->labelled;
      const int32_t label = (size * kLabelPercent + 50) / 100;
      setRect(labels, static_cast<int32_t>(node),
              labelled ? x - label / 4 : static_cast<int32_t>(kOffScreen), y - label / 4, label,
              label);
    }
    for (uint32_t lone = 0; lone < kLoneLabels; ++lone) {
      const SlotLabel &label = kSlotLabels[lone];
      setRect(labels, static_cast<int32_t>(kSlotCount + lone),
              label.device == device ? label.x + offset : static_cast<int32_t>(kOffScreen), label.y,
              label.size, label.size);
    }
    const auto setAlpha = guest::function<SetAlphaFn>(kHudItemButtonsSetAlpha);
    for (uint32_t quad = 0; quad < kStoneQuads; ++quad) {
      setAlpha(stones, static_cast<int32_t>(quad), alpha);
    }
    for (uint32_t node = 0; node < kSlotCount; ++node) {
      setAlpha(items, static_cast<int32_t>(node), alpha);
    }
    for (uint32_t quad = 0; quad < kLabelQuads; ++quad) {
      setAlpha(labels, static_cast<int32_t>(quad), alpha);
    }
    const auto updateAmmo = guest::function<LayoutFn>(kHudItemButtonsUpdateAmmo);
    updateAmmo(stones);
    updateAmmo(items);
    updateAmmo(labels);
  }

  void draw() const {
    const auto draw = guest::function<LayoutFn>(kHudItemButtonsDraw);
    draw(reinterpret_cast<void *>(stones_));
    draw(reinterpret_cast<void *>(items_));
    draw(reinterpret_cast<void *>(labels_));
  }

private:
  // HudItemButtons_SetRect sizes a count for a 42 pixel icon; a slot's goes at its stone's corner,
  // the ones rightmost, with `right` and `bottom` the stone's edges.
  void placeCount(uint32_t node, float right, float bottom, float stone) const {
    const auto count =
        guest::read<uintptr_t>(items_ + kHudItemButtonsCounts + node * sizeof(uint32_t));
    if (count == 0) {
      return;
    }
    const float height = stone * kDigitShare;
    const float size[2] = {height * kDigitAspect, height};
    const auto digits = guest::read<int32_t>(count + kHudCountDigits);
    for (int32_t digit = 0; digit < digits; ++digit) {
      const float corner[2] = {right - static_cast<float>(digit + 1) * size[0],
                               bottom - height + height * kDigitHang};
      guest::function<SetRectsFn>(kQuadBatchSetRects)(guest::read<void *>(count + kHudCountQuads),
                                                      corner, size, 1, digit);
    }
  }

  // A layout with counts: one without draws its quads blended as the icons' plain white.
  static uintptr_t layout(uint32_t count) {
    void *memory = guest::function<NewFn>(kHeapNew)(kHudItemButtonsSize);
    return reinterpret_cast<uintptr_t>(guest::function<InitFn>(kHudItemButtonsInit)(
        memory, static_cast<int32_t>(count), kItemsKind, 0));
  }

  // A layout's quads drawn from `texture`, `width` by `height` pixels.
  static void sheet(uintptr_t layout, void *texture, float width, float height) {
    auto *size = reinterpret_cast<volatile float *>(quadsOf(layout) + kQuadBatchTextureSize);
    size[0] = width;
    size[1] = height;
    guest::function<SetTextureFn>(kMenuModelSetTexture)(
        guest::read<void *>(layout + kHudItemButtonsModel), 0, texture, kLinear, kLinear,
        kClampToEdge, kClampToEdge);
  }

  uintptr_t stones_ = 0;
  uintptr_t items_ = 0;
  uintptr_t labels_ = 0;
  int32_t columnWidth_ = 0;
  int32_t columnHeight_ = 0;
};

constinit SlotColumn slotColumn;

} // namespace

// ItemMenu_Init's last call (hooks.s item_menu_init): the screen is built.
void resetItemMenu() {
  guest::function<ResetFn>(kItemMenuReset)();
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
  // The column fades with the grid: its first icon's alpha.
  slotColumn.update(
      guest::read<float>(guest::read<uintptr_t>(quadsOf(layout) + kQuadBatchColors) + kAlpha),
      -kWideMargin);
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

} // namespace z3dmods::oot3d
