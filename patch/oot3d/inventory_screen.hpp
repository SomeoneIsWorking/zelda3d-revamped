// Our inventory: the item screen's grid in our panel and cells, its tabs down the left edge
// (inventory_screen.cpp).
#pragma once

namespace z3dmods::oot3d {

// After ItemMenu_Init has built the item screen.
void buildInventoryScreen();
// Each frame of the item screen, at the screen's `alpha`.
void updateInventoryScreen(float alpha);
// Before the item screen's model (pause_screens.cpp drawItemMenu).
void drawInventoryScreen();

} // namespace z3dmods::oot3d
