// Play_Update overridden (play_update.cpp): what runs once per play frame before the game's own.
#pragma once

namespace z3dmods::oot3d {

void updateSlotItems(void *play);
void updateAutosave(void *play);
// Inside the game's Play_Update.
bool playUpdating();

} // namespace z3dmods::oot3d
