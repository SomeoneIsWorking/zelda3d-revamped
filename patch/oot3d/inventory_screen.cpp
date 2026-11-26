// Our inventory. The item screen keeps its book, BACK, tabs, cursor and items, drawn by its model
// and layouts over two layouts of ours from the screen's sheet (menu_pieces.hpp, drawn by
// tools/z3dmods/menu_art.py): a slate panel nine-sliced around the grid and a rounded cell under
// each of its items. hooks.s moves the screen's own panel and tabs' bar off the screen, BACK into
// the bottom left corner and the grid between the tabs and the slot column; the tabs are turned a
// quarter clockwise down the left edge here, their open sides on the screen's edge, and the
// screen's hit tests of BACK and the tabs ask where they are drawn.
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
using TouchInRectFn = int32_t (*)(int32_t x, int32_t y, int32_t width, int32_t height,
                                  int32_t phase);

// A rect in the touch screen's pixels.
struct Rect {
  int32_t x;
  int32_t y;
  int32_t width;
  int32_t height;
};

// A point in the touch screen's pixels.
struct Point {
  float x;
  float y;
};

// A quad of the item screen's tables, its place and size.
struct Box {
  float x;
  float y;
  float width;
  float height;
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
inline constexpr int32_t kCellMargin = 3;
// The tabs: Gear, Map and Items, each a face and three frame quads (decomp pause.md), down the
// left edge from this row.
inline constexpr uint32_t kTabs = 3;
inline constexpr uint32_t kTabFrames = 3;
inline constexpr float kTabsTop = 6.0F;
// A quad's UVs in QuadBatch.uvs, two floats a corner: top left, top right, bottom left, bottom
// right, as its corners.
inline constexpr uint32_t kUvFloats = 2;
// A quad's four corners, three floats each, in QuadBatch.corners.
inline constexpr uint32_t kQuadCorners = 4;
inline constexpr uint32_t kCornerFloats = 3;
Box tableQuad(uint32_t quad) {
  const auto *at = reinterpret_cast<const float *>(kItemMenuQuadPositions);
  const auto *size = reinterpret_cast<const float *>(kItemMenuQuadSizes);
  return {at[2 * quad], at[2 * quad + 1], size[2 * quad], size[2 * quad + 1]};
}

// Quad `quad` of a tab whose quads span `tab` in the tables, turned a quarter clockwise about the
// tab with its turned top left at `at`: its bottom, the open side, faces left.
void turnQuad(uintptr_t batch, uint32_t quad, const Box &tab, Point at) {
  const Box was = tableQuad(quad);
  const float x = at.x + tab.y + tab.height - was.y - was.height;
  const float y = at.y + was.x - tab.x;
  auto *corners =
      reinterpret_cast<volatile float *>(guest::read<uintptr_t>(batch + kQuadBatchCorners) +
                                         quad * kQuadCorners * kCornerFloats * sizeof(float));
  const float places[kQuadCorners][2] = {
      {x, y}, {x + was.height, y}, {x, y + was.width}, {x + was.height, y + was.width}};
  for (uint32_t corner = 0; corner < kQuadCorners; ++corner) {
    corners[corner * kCornerFloats] = places[corner][0];
    corners[corner * kCornerFloats + 1] = places[corner][1];
  }
  auto *uvs = reinterpret_cast<volatile float *>(guest::read<uintptr_t>(batch + kQuadBatchUvs) +
                                                 quad * kQuadCorners * kUvFloats * sizeof(float));
  // Each turned corner shows the texel of the corner turned onto it.
  constexpr uint32_t kFrom[kQuadCorners] = {2, 0, 3, 1};
  float wasUvs[kQuadCorners * kUvFloats];
  for (uint32_t at = 0; at < kQuadCorners * kUvFloats; ++at) {
    wasUvs[at] = uvs[at];
  }
  for (uint32_t corner = 0; corner < kQuadCorners; ++corner) {
    uvs[corner * kUvFloats] = wasUvs[kFrom[corner] * kUvFloats];
    uvs[corner * kUvFloats + 1] = wasUvs[kFrom[corner] * kUvFloats + 1];
  }
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
    panel_.create(kSlices * kSlices);
    cells_.create(kCells);
    each([&](const MenuLayout &layout) {
      layout.sheet(sheet, sheetSize[0], sheetSize[1]);
    });
    turnTabs(screen);
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

  // The game's hit test of BACK or a tab, its rect where the game drew it: where we draw it, the
  // part on the touch screen; a press beside the touch screen touches its edge (the app).
  [[nodiscard]] int32_t touched(const Rect &asked, int32_t phase) const {
    Box drawn{static_cast<float>(asked.x), static_cast<float>(asked.y),
              static_cast<float>(asked.width), static_cast<float>(asked.height)};
    bool tab = false;
    for (uint32_t index = 0; index < kTabs; ++index) {
      if (static_cast<int32_t>(tableQuad(kItemMenuQuadTabs + index).x) == asked.x) {
        drawn = tabs_[index];
        tab = true;
      }
    }
    if (!tab) {
      const Box back = tableQuad(kItemMenuQuadBack);
      const Box end = tableQuad(kItemMenuQuadBack + 1);
      drawn = {back.x, back.y, end.x + end.width - back.x, back.height};
    }
    const float left = drawn.x > 0.0F ? drawn.x : 0.0F;
    const float right = drawn.x + drawn.width > 0.0F ? drawn.x + drawn.width : 0.0F;
    return guest::function<TouchInRectFn>(kTouchInRect)(
        static_cast<int32_t>(left), static_cast<int32_t>(drawn.y),
        static_cast<int32_t>(right - left), static_cast<int32_t>(drawn.height), phase);
  }

private:
  // Each tab's face and frames turned down the left edge, in the order they ran along the bottom.
  void turnTabs(uintptr_t batch) {
    const float first = tableQuad(kItemMenuQuadTabs).x;
    for (uint32_t index = 0; index < kTabs; ++index) {
      const uint32_t face = kItemMenuQuadTabs + index;
      const Box tab = tableQuad(face);
      const float left = -kWideMargin;
      const float top = kTabsTop + tab.x - first;
      turnQuad(batch, face, tab, {left, top});
      for (uint32_t frame = 0; frame < kTabFrames; ++frame) {
        turnQuad(batch, kItemMenuQuadTabFrames + index * kTabFrames + frame, tab, {left, top});
      }
      tabs_[index] = {left, top, tab.height, tab.width};
    }
  }

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

  Box tabs_[kTabs] = {};
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
