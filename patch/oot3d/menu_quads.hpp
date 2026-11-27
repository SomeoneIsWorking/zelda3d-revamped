// A menu's quads moved after the game builds them: their places from the game's own tables,
// shifted, and set again into the built batch with QuadBatch_SetRects (decomp docs/pause.md,
// docs/file_select.md).
#pragma once

#include "game.hpp"

#include <stdint.h>

namespace z3dmods::oot3d {

// Far enough across that a quad is off every screen.
inline constexpr float kOffScreen = 1000.0F;
// BACK in the top screen's corner, past the touch screen's left edge.
inline constexpr float kCorneredBack = -kWideMargin;

// A menu's tables of its quads' places and sizes, x and y, width and height.
struct QuadTables {
  uintptr_t positions;
  uintptr_t sizes;
};

// A run of a batch's quads.
struct QuadRange {
  uint32_t first;
  uint32_t count;
};

// `quads` of `batch`, built from `tables`, moved across by `dx`.
void shiftQuads(uintptr_t batch, const QuadTables &tables, QuadRange quads, float dx);

} // namespace z3dmods::oot3d
