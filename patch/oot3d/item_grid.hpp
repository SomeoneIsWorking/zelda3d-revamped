// The item screen's grid between the tabs down the left edge and the slot column: where its nodes
// sit, and the game's cursor and stylus on it (item_grid.cpp).
#pragma once

#include "symbols.hpp"

#include <stdint.h>

namespace z3dmods::oot3d {

// The grid's columns; ItemMenu's last column is the buttons', which the slot column replaces.
inline constexpr int32_t kGridColumns = kItemMenuColumns - 1;

// ItemMenu_NodePosition, overridden: node's place, its icon's top left.
void itemNodePosition(int node, float *x, float *y);
// ItemMenu_PlaceCursor, overridden: the cursor's corners aimed at its node.
void placeItemCursor();
// ItemMenu_UpdateCursor, overridden: the pad moves the cursor and picks, moves and assigns items.
void updateItemCursor();
// ItemMenu_UpdateTouch, overridden: the stylus does the same; `phase` 1 drops, 2 drags. Not 0
// while it took the touch.
int updateItemTouch(int phase);

} // namespace z3dmods::oot3d
