// The pause screens' tabs (Gear, Map, Items) turned a quarter clockwise down the left edge, their
// open sides on the screen's edge, and the screens' hit tests of BACK and the tabs asked where
// they are drawn (decomp docs/pause.md). Each screen layout's batch is turned once, after the game
// builds it from its tables.
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

class PauseTabs {
public:
  // Turns the tabs of `batch`, built from `layout`'s tables.
  void turn(uintptr_t batch, const PauseLayout &layout);
  // A screen's hit test of BACK or a tab, Touch_InRect's rect where the game drew it: where we
  // draw it, the part on the touch screen; the app touches the edge for a press beside it.
  [[nodiscard]] int32_t touched(const Rect &asked, int32_t phase) const;

private:
  struct Box {
    float x;
    float y;
    float width;
    float height;
  };
  static constexpr uint32_t kTabs = 3;

  [[nodiscard]] static Box tableQuad(const PauseLayout &layout, uint32_t quad);
  static void turnQuad(uintptr_t batch, const PauseLayout &layout, uint32_t quad, const Box &tab,
                       const Box &turned);

  PauseLayout layout_{};
  Box tabs_[kTabs] = {};
};

} // namespace z3dmods::oot3d
