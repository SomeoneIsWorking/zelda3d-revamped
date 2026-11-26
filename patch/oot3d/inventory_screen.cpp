// Our inventory. The item screen keeps its own BACK, tabs, cursor and items, drawn by its model
// and layouts over three layouts of ours from the screen's sheet (menu_pieces.hpp, drawn by
// tools/z3dmods/menu_art.py): a dark backdrop over the whole widened screen in place of the book,
// a slate panel nine-sliced where the screen's own panel was (hooks.s item_panel moves that and
// the tabs' bar off the screen), and a rounded cell under each of the grid's items.
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
// The panel's nine slices: its corners' side, in the menu's pixels, and its inset from the
// screen's panel at the top and bottom, clear of the screen's edge and the tabs.
inline constexpr int32_t kSlices = 3;
inline constexpr int32_t kPanelCorner = 12;
inline constexpr int32_t kPanelInset = 1;
// How far a cell reaches past its item, a side.
inline constexpr int32_t kCellMargin = 1;
// A quad's four corners, three floats each, in QuadBatch.corners.
inline constexpr uint32_t kQuadCorners = 4;
inline constexpr uint32_t kCornerFloats = 3;
// The backdrop's cell is sampled this far inside its sides, clear of its neighbours.
inline constexpr float kBackdropInset = 1.0F;
// Off the screen by the item_panel patch.
inline constexpr float kMovedOff = 1000.0F;

// Quad `quad` of the item screen's tables, where the game put it.
Rect screenQuad(uint32_t quad) {
  const auto *at = reinterpret_cast<const float *>(kItemMenuQuadPositions);
  const auto *size = reinterpret_cast<const float *>(kItemMenuQuadSizes);
  return {static_cast<int32_t>(at[2 * quad] - kMovedOff), static_cast<int32_t>(at[2 * quad + 1]),
          static_cast<int32_t>(size[2 * quad]), static_cast<int32_t>(size[2 * quad + 1])};
}

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
    buildPanel();
    for (uint32_t cell = 0; cell < kCells; ++cell) {
      cells_.cell(cell, {kMenuPieceCell.u, kMenuPieceCell.v},
                  {kMenuPieceCell.width, kMenuPieceCell.height});
    }
    cellsPlaced_ = false;
  }

  // The grid's places are set in the screen's first update, so its cells go there then.
  void update(float alpha) {
    if (!cellsPlaced_ && itemRect(0).width > 0) {
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

  // Over the screen's panel, its two halves: each slice of the piece stretched between its
  // corners.
  void buildPanel() const {
    const Rect leftHalf = screenQuad(kItemMenuQuadPanel);
    const Rect rightHalf = screenQuad(kItemMenuQuadPanel + 1);
    const int32_t top = leftHalf.y + kPanelInset;
    const int32_t bottom = leftHalf.y + leftHalf.height - kPanelInset;
    const int32_t right = rightHalf.x + rightHalf.width;
    const int32_t xs[] = {leftHalf.x, leftHalf.x + kPanelCorner, right - kPanelCorner, right};
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
