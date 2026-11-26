// Our inventory. The item screen keeps its book, BACK, tabs, cursor and items, drawn by its model
// and layouts over two layouts of ours from the screen's sheet (menu_pieces.hpp, drawn by
// tools/z3dmods/menu_art.py): a slate panel nine-sliced around the grid and a rounded cell under
// each of its items. hooks.s moves the screen's own panel and tabs' bar off the screen, BACK into
// the bottom left corner and the grid between the tabs and the slot column; its tabs go down the
// left edge (pause_tabs.hpp).
#include "inventory_screen.hpp"

#include "../common/guest.hpp"
#include "game.hpp"
#include "menu_layout.hpp"
#include "menu_pieces.hpp"
#include "pause_tabs.hpp"
#include "symbols.hpp"

#include <stdint.h>

namespace z3dmods::oot3d {
namespace {

using TextureGetFn = void *(*)(int32_t slot);

// The grid's items, without the column of buttons ItemMenu's cursor also reaches.
inline constexpr int32_t kGridColumns = kItemMenuColumns - 1;
inline constexpr uint32_t kCells = kGridColumns * kItemMenuRows;
// The panel's nine slices: its corners' side and how far it reaches past the grid, in the menu's
// pixels.
inline constexpr int32_t kSlices = 3;
inline constexpr int32_t kPanelCorner = 12;
inline constexpr int32_t kPanelPad = 6;
// How far a cell reaches past its item, a side.
inline constexpr int32_t kCellMargin = 3;

// The grid's item `node`, from its icon's corners; empty until the screen's first update.
Rect itemRect(int32_t node) {
  const auto items = guest::read<uintptr_t>(kItemMenu + kItemMenuItems);
  const auto batch = guest::read<uintptr_t>(items + kHudItemButtonsQuads);
  const auto *corners =
      reinterpret_cast<const volatile float *>(guest::read<uintptr_t>(batch + kQuadBatchCorners) +
                                               node * kQuadCorners * kCornerFloats * sizeof(float));
  float left = corners[0];
  float top = corners[1];
  float right = left;
  float bottom = top;
  for (uint32_t corner = 1; corner < kQuadCorners; ++corner) {
    const float x = corners[corner * kCornerFloats];
    const float y = corners[corner * kCornerFloats + 1];
    left = x < left ? x : left;
    right = x > right ? x : right;
    top = y < top ? y : top;
    bottom = y > bottom ? y : bottom;
  }
  return {static_cast<int32_t>(left), static_cast<int32_t>(top), static_cast<int32_t>(right - left),
          static_cast<int32_t>(bottom - top)};
}

class InventoryScreen {
public:
  void build() {
    const auto screen = guest::read<uintptr_t>(kItemMenu + kItemMenuQuads);
    const auto *sheetSize =
        reinterpret_cast<const volatile float *>(screen + kQuadBatchTextureSize);
    void *sheet = guest::function<TextureGetFn>(kMenuTextureGet)(kMenuTextureItemParts);
    panel_.create(kSlices * kSlices);
    cells_.create(kCells);
    each([&](const MenuLayout &layout) {
      layout.sheet(sheet, sheetSize[0], sheetSize[1]);
    });
    tabs_.turn(screen, {kItemMenuQuadPositions, kItemMenuQuadSizes, kItemMenuQuadTabs,
                        kItemMenuQuadTabFrames, kItemMenuQuadBack});
    for (uint32_t cell = 0; cell < kCells; ++cell) {
      cells_.cell(cell, {kMenuPieceCell.u, kMenuPieceCell.v},
                  {kMenuPieceCell.width, kMenuPieceCell.height});
    }
    cellsPlaced_ = false;
  }

  // The grid's places are set in the screen's first update, so its panel and cells go there then.
  void update(float alpha) {
    if (!cellsPlaced_ && itemRect(0).width > 0) {
      placePanel();
      for (int32_t row = 0; row < kItemMenuRows; ++row) {
        for (int32_t column = 0; column < kGridColumns; ++column) {
          const Rect item = itemRect(row * kItemMenuColumns + column);
          cells_.place(static_cast<uint32_t>(row * kGridColumns + column), item.x - kCellMargin,
                       item.y - kCellMargin, item.width + 2 * kCellMargin,
                       item.height + 2 * kCellMargin);
        }
      }
      cellsPlaced_ = true;
    }
    each([&](const MenuLayout &layout) {
      layout.fade(alpha);
    });
  }

  void draw() const {
    each([](const MenuLayout &layout) {
      layout.draw();
    });
  }

  [[nodiscard]] int32_t touched(const Rect &asked, int32_t phase) const {
    return tabs_.touched(asked, phase);
  }

private:
  // Each of ours, bottom to top.
  template <typename Fn> void each(Fn fn) const {
    fn(panel_);
    fn(cells_);
  }

  // Around the grid, from its first item to its last: each slice of the piece stretched between
  // its corners.
  void placePanel() const {
    const Rect first = itemRect(0);
    const Rect last = itemRect((kItemMenuRows - 1) * kItemMenuColumns + kGridColumns - 1);
    const int32_t left = first.x - kPanelPad;
    const int32_t top = first.y - kPanelPad;
    const int32_t right = last.x + last.width + kPanelPad;
    const int32_t bottom = last.y + last.height + kPanelPad;
    const int32_t xs[] = {left, left + kPanelCorner, right - kPanelCorner, right};
    const int32_t ys[] = {top, top + kPanelCorner, bottom - kPanelCorner, bottom};
    const float piece = kMenuPiecePanel.width;
    const float cuts[] = {0.0F, static_cast<float>(kPanelCorner), piece - kPanelCorner, piece};
    for (int32_t row = 0; row < kSlices; ++row) {
      for (int32_t column = 0; column < kSlices; ++column) {
        const auto quad = static_cast<uint32_t>(row * kSlices + column);
        panel_.cell(quad, {kMenuPiecePanel.u + cuts[column], kMenuPiecePanel.v + cuts[row]},
                    {cuts[column + 1] - cuts[column], cuts[row + 1] - cuts[row]});
        panel_.place(quad, xs[column], ys[row], xs[column + 1] - xs[column], ys[row + 1] - ys[row]);
      }
    }
  }

  PauseTabs tabs_;
  MenuLayout panel_;
  MenuLayout cells_;
  bool cellsPlaced_ = false;
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

int inventoryTouched(int x, int y, int width, int height, int phase) {
  return inventoryScreen.touched({x, y, width, height}, phase);
}

} // namespace z3dmods::oot3d
