// A pause screen's tabs (Gear, Map, Items) turned a quarter clockwise down the left edge, their
// open sides on the screen's edge, and where it draws BACK and them for its hit tests (decomp
// docs/pause.md). Each screen layout's batch is turned once, after the game builds it.
#pragma once

#include <stdint.h>

namespace z3dmods::oot3d {

// A rect in the touch screen's pixels.
struct Rect {
  int32_t x;
  int32_t y;
  int32_t width;
  int32_t height;
};

// A quad's four corners, three floats each, in QuadBatch.corners: top left, top right, bottom
// left, bottom right.
inline constexpr uint32_t kQuadCorners = 4;
inline constexpr uint32_t kCornerFloats = 3;

// A screen layout's tables and where its tabs and BACK are in them.
struct PauseLayout {
  uintptr_t positions;
  uintptr_t sizes;
  // Gear's, Map's and Items' faces from here, and their frames, three each, from `frames`.
  uint32_t faces;
  uint32_t frames;
  // BACK's two quads from here.
  uint32_t back;
};

// A place and size in the touch screen's pixels.
struct Box {
  float x;
  float y;
  float width;
  float height;
};

class PauseTabs {
public:
  // Turns the tabs of `batch`, built from `layout`'s tables.
  void turn(uintptr_t batch, const PauseLayout &layout);
  // Whether `asked`, a hit test's rect, is a tab's where the game drew it, and then where we draw
  // that tab, in `drawn`.
  [[nodiscard]] bool tab(const Rect &asked, Box &drawn) const;
  // Where we draw BACK.
  [[nodiscard]] Box back() const;

private:
  static constexpr uint32_t kTabs = 3;

  [[nodiscard]] static Box tableQuad(const PauseLayout &layout, uint32_t quad);
  static void turnQuad(uintptr_t batch, const PauseLayout &layout, uint32_t quad, const Box &tab,
                       const Box &turned);

  PauseLayout layout_{};
  Box tabs_[kTabs] = {};
};

} // namespace z3dmods::oot3d
