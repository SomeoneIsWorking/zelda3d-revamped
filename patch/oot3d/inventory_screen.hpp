// Our inventory: the item screen's grid in our panel and cells, its tabs down the left edge
// (inventory_screen.cpp).
#pragma once

namespace z3dmods::oot3d {

// After ItemMenu_Init has built the item screen.
void buildInventoryScreen();
// Each frame of the item screen, at the screen's `alpha`.
void updateInventoryScreen(float alpha);
// Before the item screen's model (hooks.s item_menu_draw_model).
void drawInventoryScreen();
// ItemMenu_NodePosition, overridden (hooks.s item_node_position): node's place, its icon's top
// left.
void itemNodePosition(int node, float *x, float *y);
// ItemMenu_PlaceCursor, overridden (hooks.s item_place_cursor): the cursor's corners aimed at
// its node.
void placeItemCursor();

} // namespace z3dmods::oot3d
