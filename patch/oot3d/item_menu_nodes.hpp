// The item screen's nodes in the save (decomp z3Dsave.hpp): each node's inventory slot, per age,
// and the item in a slot.
#pragma once

#include "../common/guest.hpp"
#include "game.hpp"
#include "symbols.hpp"

#include <stdint.h>

namespace z3dmods::oot3d {

inline bool linkIsChild() {
  return guest::read<int32_t>(kSaveContext + kSaveLinkAge) == kLinkAgeChild;
}

// The 24 nodes' inventory slots, kSlotNone for an empty node.
inline volatile uint8_t *itemMenuNodes(bool child) {
  return reinterpret_cast<volatile uint8_t *>(kSaveContext +
                                              (child ? kSaveItemMenuChild : kSaveItemMenuAdult));
}

inline uint8_t inventoryItem(uint8_t inventorySlot) {
  return guest::read<uint8_t>(kSaveContext + kSaveInventory + kInventoryItems + inventorySlot);
}

} // namespace z3dmods::oot3d
