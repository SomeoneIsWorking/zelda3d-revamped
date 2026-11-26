// The gear and map screens' tabs down the left edge (pause_screens.cpp).
#pragma once

namespace z3dmods::oot3d {

// PauseMenus_Init's calls of GearMenu_Init and MapMenu_Init (hooks.s gear_menu_init,
// map_menu_init).
void initGearMenu();
void initMapMenu();
// GearMenu_Update's and MapMenu_Update's hit tests of BACK and the tabs (hooks.s gear_*_touch*,
// map_*_touch*), Touch_InRect's arguments: whether the stylus is on it, where we draw it.
int gearTouched(int x, int y, int width, int height, int phase);
int mapTouched(int x, int y, int width, int height, int phase);

} // namespace z3dmods::oot3d
