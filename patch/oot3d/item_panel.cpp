// The item screen's panel (two quads of the screen's sheet, the right one mirrored) ends flat
// above BACK and the tabs. Trimmed of its dark last rows (hooks.s item_panel_*), it is carried on
// to the screen's bottom edge with its plain rows under the last row of slots, tiled and mirrored
// so each tile meets the next on the same row, drawn under the screen so BACK and the tabs stay
// on top.
#include "item_panel.hpp"

#include "../common/guest.hpp"
#include "game.hpp"
#include "menu_layout.hpp"
#include "symbols.hpp"

#include <stdint.h>

namespace z3dmods::oot3d {
namespace {

using TextureGetFn = void *(*)(int32_t slot);

inline constexpr uint32_t kPanelQuads = 2;
// The plain rows the foot tiles, in the cell's pixels: under the last row of slots, to where the
// trimmed cell ends.
inline constexpr int32_t kFootRows = 8;
// Enough to reach the screen's bottom edge from the trimmed panel's.
inline constexpr uint32_t kTiles = 6;
inline constexpr uint32_t kQuads = kPanelQuads * kTiles;

class ItemPanelFoot {
public:
  void build() {
    foot_.create(kQuads);
    const auto screen = guest::read<uintptr_t>(kItemMenu + kItemMenuQuads);
    const auto *screenSize =
        reinterpret_cast<const volatile float *>(screen + kQuadBatchTextureSize);
    foot_.sheet(guest::function<TextureGetFn>(kMenuTextureGet)(kMenuTextureItemParts),
                screenSize[0], screenSize[1]);
    const auto *cells = reinterpret_cast<const float *>(kItemMenuQuadUvs);
    const auto *cellSizes = reinterpret_cast<const float *>(kItemMenuQuadUvSizes);
    const auto *corners = reinterpret_cast<const float *>(kItemMenuQuadPositions);
    const auto *sizes = reinterpret_cast<const float *>(kItemMenuQuadSizes);
    for (uint32_t half = 0; half < kPanelQuads; ++half) {
      const uint32_t quad = kItemMenuQuadPanel + half;
      const float end = cells[2 * quad + 1] + cellSizes[2 * quad + 1];
      const auto x = static_cast<int32_t>(corners[2 * quad]);
      const auto width = static_cast<int32_t>(sizes[2 * quad]);
      int32_t y = static_cast<int32_t>(corners[2 * quad + 1] + sizes[2 * quad + 1]);
      for (uint32_t tile = 0; tile < kTiles; ++tile) {
        const uint32_t at = half * kTiles + tile;
        // The first tile mirrors the rows above it, the next the first, and so on.
        const bool mirrored = tile % 2 == 0;
        const float rows = static_cast<float>(kFootRows);
        foot_.cell(at, {cells[2 * quad], mirrored ? end : end - rows},
                   {cellSizes[2 * quad], mirrored ? -rows : rows});
        foot_.place(at, x, y, width, kFootRows);
        y += kFootRows;
      }
    }
  }

  void update(float alpha) const {
    foot_.fade(alpha);
  }

  void draw() const {
    foot_.draw();
  }

private:
  MenuLayout foot_;
};

constinit ItemPanelFoot itemPanelFoot;

} // namespace

void buildItemPanelFoot() {
  itemPanelFoot.build();
}

void updateItemPanelFoot(float alpha) {
  itemPanelFoot.update(alpha);
}

void drawItemPanelFoot() {
  itemPanelFoot.draw();
}

} // namespace z3dmods::oot3d
