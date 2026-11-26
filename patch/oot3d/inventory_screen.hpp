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
// ItemMenu_Update's hit tests of BACK and the tabs (hooks.s item_*_touch*), Touch_InRect's
// arguments: whether the stylus is on it, where we draw it.
int inventoryTouched(int x, int y, int width, int height, int phase);

} // namespace z3dmods::oot3d
