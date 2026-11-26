#include "pause_tabs.hpp"

#include "../common/guest.hpp"
#include "game.hpp"
#include "symbols.hpp"

namespace z3dmods::oot3d {
namespace {

using TouchInRectFn = int32_t (*)(int32_t x, int32_t y, int32_t width, int32_t height,
                                  int32_t phase);

// Each tab's face and three frames.
inline constexpr uint32_t kTabFrames = 3;
// The first tab's turned top.
inline constexpr float kTabsTop = 6.0F;
// A quad's UVs in QuadBatch.uvs, two floats a corner, in its corners' order.
inline constexpr uint32_t kUvFloats = 2;
// Each turned corner shows the texel of the corner turned onto it.
inline constexpr uint32_t kTurnedFrom[kQuadCorners] = {2, 0, 3, 1};

} // namespace

PauseTabs::Box PauseTabs::tableQuad(const PauseLayout &layout, uint32_t quad) {
  const auto *at = reinterpret_cast<const float *>(layout.positions);
  const auto *size = reinterpret_cast<const float *>(layout.sizes);
  return {at[2 * quad], at[2 * quad + 1], size[2 * quad], size[2 * quad + 1]};
}

// Quad `quad` of a tab whose quads span `tab` in the tables, turned with the tab onto `turned`:
// the tab's bottom, its open side, faces left.
void PauseTabs::turnQuad(uintptr_t batch, const PauseLayout &layout, uint32_t quad, const Box &tab,
                         const Box &turned) {
  const Box was = tableQuad(layout, quad);
  const float x = turned.x + tab.y + tab.height - was.y - was.height;
  const float y = turned.y + was.x - tab.x;
  auto *corners =
      reinterpret_cast<volatile float *>(guest::read<uintptr_t>(batch + kQuadBatchCorners) +
                                         quad * kQuadCorners * kCornerFloats * sizeof(float));
  const float places[kQuadCorners][2] = {
      {x, y}, {x + was.height, y}, {x, y + was.width}, {x + was.height, y + was.width}};
  for (uint32_t corner = 0; corner < kQuadCorners; ++corner) {
    corners[corner * kCornerFloats] = places[corner][0];
    corners[corner * kCornerFloats + 1] = places[corner][1];
  }
  auto *uvs = reinterpret_cast<volatile float *>(guest::read<uintptr_t>(batch + kQuadBatchUvs) +
                                                 quad * kQuadCorners * kUvFloats * sizeof(float));
  float wasUvs[kQuadCorners * kUvFloats];
  for (uint32_t at = 0; at < kQuadCorners * kUvFloats; ++at) {
    wasUvs[at] = uvs[at];
  }
  for (uint32_t corner = 0; corner < kQuadCorners; ++corner) {
    uvs[corner * kUvFloats] = wasUvs[kTurnedFrom[corner] * kUvFloats];
    uvs[corner * kUvFloats + 1] = wasUvs[kTurnedFrom[corner] * kUvFloats + 1];
  }
}

// In the order they ran along the bottom, from the left edge.
void PauseTabs::turn(uintptr_t batch, const PauseLayout &layout) {
  layout_ = layout;
  const float first = tableQuad(layout, layout.faces).x;
  for (uint32_t index = 0; index < kTabs; ++index) {
    const Box tab = tableQuad(layout, layout.faces + index);
    const Box turned{-kWideMargin, kTabsTop + tab.x - first, tab.height, tab.width};
    turnQuad(batch, layout, layout.faces + index, tab, turned);
    for (uint32_t frame = 0; frame < kTabFrames; ++frame) {
      turnQuad(batch, layout, layout.frames + index * kTabFrames + frame, tab, turned);
    }
    tabs_[index] = turned;
  }
}

int32_t PauseTabs::touched(const Rect &asked, int32_t phase) const {
  const Box back = tableQuad(layout_, layout_.back);
  const Box end = tableQuad(layout_, layout_.back + 1);
  Box drawn{back.x, back.y, end.x + end.width - back.x, back.height};
  for (uint32_t index = 0; index < kTabs; ++index) {
    if (static_cast<int32_t>(tableQuad(layout_, layout_.faces + index).x) == asked.x) {
      drawn = tabs_[index];
    }
  }
  const float left = drawn.x > 0.0F ? drawn.x : 0.0F;
  const float right = drawn.x + drawn.width > 0.0F ? drawn.x + drawn.width : 0.0F;
  return guest::function<TouchInRectFn>(kTouchInRect)(
      static_cast<int32_t>(left), static_cast<int32_t>(drawn.y), static_cast<int32_t>(right - left),
      static_cast<int32_t>(drawn.height), phase);
}

} // namespace z3dmods::oot3d
