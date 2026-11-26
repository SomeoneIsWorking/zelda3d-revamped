// The pause screens' tabs down the left edge and the stylus's hit tests of BACK and the tabs
// where they are drawn (pause_screens.cpp).
#pragma once

#include <stdint.h>

namespace z3dmods::oot3d {

// The item screen's batch, once ItemMenu_Init has built it (inventory_screen.cpp).
void turnItemTabs(uintptr_t batch);
// PauseMenus_Init's calls of GearMenu_Init and MapMenu_Init (hooks.s gear_menu_init,
// map_menu_init).
void initGearMenu();
void initMapMenu();
// Touch_InRect, overridden (hooks.s touch_in_rect): whether the stylus is in the rect in the
// phase; while a pause screen is open, BACK's and its tabs' rects are where we draw them.
int touchInRect(int x, int y, int width, int height, int phase);

} // namespace z3dmods::oot3d
