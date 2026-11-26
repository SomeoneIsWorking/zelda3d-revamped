// Our inventory. The item screen keeps its own BACK, tabs, cursor and items, drawn by its model
// and layouts over three layouts of ours from the screen's sheet (menu_pieces.hpp, drawn by
// tools/z3dmods/menu_art.py): a dark backdrop over the whole widened screen in place of the book,
// a slate panel nine-sliced around the grid, and a rounded cell under each of the grid's items.
// hooks.s moves the screen's own panel and tabs' bar off the screen, its BACK to the top left
// corner, its tabs along the top and the grid down under them.
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

// A rect in the touch screen's pixels.
struct Rect {
  int32_t x;
  int32_t y;
  int32_t width;
  int32_t height;
};

// The grid's items, without the column of buttons ItemMenu's cursor also reaches.
inline constexpr int32_t kGridColumns = kItemMenuColumns - 1;
inline constexpr uint32_t kCells = kGridColumns * kItemMenuRows;
// The panel's nine slices: its corners' side and how far it reaches past the grid, in the menu's
// pixels.
inline constexpr int32_t kSlices = 3;
inline constexpr int32_t kPanelCorner = 12;
inline constexpr int32_t kPanelPad = 6;
// How far a cell reaches past its item, a side.
inline constexpr int32_t kCellMargin = 1;
// A quad's four corners, three floats each, in QuadBatch.corners.
inline constexpr uint32_t kQuadCorners = 4;
inline constexpr uint32_t kCornerFloats = 3;
// The backdrop's cell is sampled this far inside its sides, clear of its neighbours.
inline constexpr float kBackdropInset = 1.0F;
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
    backdrop_.create(1);
    panel_.create(kSlices * kSlices);
    cells_.create(kCells);
    each([&](const MenuLayout &layout) {
      layout.sheet(sheet, sheetSize[0], sheetSize[1]);
    });
    backdrop_.cell(0, {kMenuPieceBackdrop.u + kBackdropInset, kMenuPieceBackdrop.v},
                   {kMenuPieceBackdrop.width - 2 * kBackdropInset, kMenuPieceBackdrop.height});
    backdrop_.place(0, -static_cast<int32_t>(kWideMargin), 0, static_cast<int32_t>(kTopWidth),
                    static_cast<int32_t>(kTouchHeight));
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

private:
  // Each of ours, bottom to top.
  template <typename Fn> void each(Fn fn) const {
    fn(backdrop_);
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

  MenuLayout backdrop_;
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

} // namespace z3dmods::oot3d
