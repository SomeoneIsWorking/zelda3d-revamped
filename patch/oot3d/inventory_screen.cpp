// Our inventory. The item screen keeps its own art (its panel and slots, BACK, the tabs, the
// cursor), drawn by its model over two layouts of ours from the screen's sheet: a dark backdrop
// over the whole widened screen in place of the book (menu_pieces.hpp, drawn by
// tools/z3dmods/menu_art.py), and the panel's foot. The panel ends flat above BACK and the tabs;
// trimmed of its dark last rows (hooks.s item_panel_*), it is carried on to the screen's bottom
// edge with its plain rows under the last row of slots, tiled and mirrored so each tile meets the
// next on the same row, and BACK and the tabs sit on it.
#include "inventory_screen.hpp"

#include "../common/guest.hpp"
#include "game.hpp"
#include "menu_layout.hpp"
#include "menu_pieces.hpp"
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
// The backdrop's cell is sampled this far inside its sides, clear of its neighbours.
inline constexpr float kBackdropInset = 1.0F;

class InventoryScreen {
public:
  void build() {
    const auto screen = guest::read<uintptr_t>(kItemMenu + kItemMenuQuads);
    const auto *sheetSize =
        reinterpret_cast<const volatile float *>(screen + kQuadBatchTextureSize);
    void *sheet = guest::function<TextureGetFn>(kMenuTextureGet)(kMenuTextureItemParts);
    backdrop_.create(1);
    foot_.create(kPanelQuads * kTiles);
    backdrop_.sheet(sheet, sheetSize[0], sheetSize[1]);
    foot_.sheet(sheet, sheetSize[0], sheetSize[1]);
    backdrop_.cell(0, {kMenuPieceBackdrop.u + kBackdropInset, kMenuPieceBackdrop.v},
                   {kMenuPieceBackdrop.width - 2 * kBackdropInset, kMenuPieceBackdrop.height});
    backdrop_.place(0, -static_cast<int32_t>(kWideMargin), 0, static_cast<int32_t>(kTopWidth),
                    static_cast<int32_t>(kTouchHeight));
    buildFoot();
  }

  void update(float alpha) const {
    backdrop_.fade(alpha);
    foot_.fade(alpha);
  }

  void draw() const {
    backdrop_.draw();
    foot_.draw();
  }

private:
  void buildFoot() const {
    const auto *cells = reinterpret_cast<const float *>(kItemMenuQuadUvs);
    const auto *cellSizes = reinterpret_cast<const float *>(kItemMenuQuadUvSizes);
    const auto *corners = reinterpret_cast<const float *>(kItemMenuQuadPositions);
    const auto *sizes = reinterpret_cast<const float *>(kItemMenuQuadSizes);
    const auto rows = static_cast<float>(kFootRows);
    for (uint32_t half = 0; half < kPanelQuads; ++half) {
      const uint32_t quad = kItemMenuQuadPanel + half;
      const float end = cells[2 * quad + 1] + cellSizes[2 * quad + 1];
      const auto x = static_cast<int32_t>(corners[2 * quad]);
      const auto width = static_cast<int32_t>(sizes[2 * quad]);
      auto y = static_cast<int32_t>(corners[2 * quad + 1] + sizes[2 * quad + 1]);
      for (uint32_t tile = 0; tile < kTiles; ++tile) {
        const uint32_t at = half * kTiles + tile;
        // The first tile mirrors the rows above it, the next the first, and so on.
        const bool mirrored = tile % 2 == 0;
        foot_.cell(at, {cells[2 * quad], mirrored ? end : end - rows},
                   {cellSizes[2 * quad], mirrored ? -rows : rows});
        foot_.place(at, x, y, width, kFootRows);
        y += kFootRows;
      }
    }
  }

  MenuLayout backdrop_;
  MenuLayout foot_;
};

constinit InventoryScreen inventoryScreen;

} // namespace

void buildInventoryScreen() {
  inventoryScreen.build();
}

void updateInventoryScreen(float alpha) {
  inventoryScreen.update(alpha);
}

void drawInventoryScreen() {
  inventoryScreen.draw();
}

} // namespace z3dmods::oot3d
