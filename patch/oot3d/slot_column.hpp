// The item screen's column of number-key slots, drawn by the game (slot_column.cpp).
#pragma once

namespace z3dmods::oot3d {

// Once the item screen is built (pause_screens.cpp initItemMenu).
void buildSlotColumn();
// HudItemButtons_UpdateAmmo, overridden (hooks.s): on the item screen's grid, its button nodes
// off the screen and the column updated.
void updateItemButtons(void *buttons);
// Under the item screen's layout at +0x40 (pause_screens.cpp drawItemMenu).
void drawSlotColumn();

} // namespace z3dmods::oot3d
