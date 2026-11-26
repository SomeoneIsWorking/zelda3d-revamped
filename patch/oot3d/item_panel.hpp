// The item screen's panel carried on below its art, under BACK and the tabs, to the screen's
// bottom edge (item_panel.cpp).
#pragma once

namespace z3dmods::oot3d {

// After ItemMenu_Init has built the item screen.
void buildItemPanelFoot();
// Each frame of the item screen, at the screen's `alpha`.
void updateItemPanelFoot(float alpha);
// Before the item screen's model (hooks.s item_menu_draw_model).
void drawItemPanelFoot();

} // namespace z3dmods::oot3d
