// The pause screens built our way and the stylus's hit tests of BACK and the tabs where they are
// drawn (pause_screens.cpp).
#pragma once

namespace z3dmods::oot3d {

// ItemMenu_Init, GearMenu_Init, MapMenu_Init and SaveMenu_Init, overridden (hooks.s): the game's
// build, then our layout.
void initItemMenu();
void initGearMenu();
void initMapMenu();
void initSaveMenu();
// Touch_InRect, overridden (hooks.s touch_in_rect): whether the stylus is in the rect in the
// phase; while a pause screen is open, BACK's and its tabs' rects are where we draw them.
int touchInRect(int x, int y, int width, int height, int phase);

} // namespace z3dmods::oot3d
