// A touch screen's page drawn on the top screen (decomp docs/ui.md, docs/items.md): its frame, the
// place and scale its touch pixels take there, and what keeps a part of it at full size.
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace z3dmods::mm3d {

struct PageFrame {
  // Where touch pixel (0, 0) goes, and the scale.
  float left;
  float top;
  float scale;

  // A touch pixel's place on the top screen.
  [[nodiscard]] constexpr float x(float touchX) const {
    return left + scale * touchX;
  }
  [[nodiscard]] constexpr float y(float touchY) const {
    return top + scale * touchY;
  }

  // The page's root.
  void place(void *layout) const;
  // A child with its own layout, whose touch point (touchX, touchY) is drawn at full size at
  // (toX, toY). Its own layout's root carries it, since the game animates the child's translate.
  void unscale(uint8_t *child, float touchX, float touchY, float toX, float toY) const;
  // A touch area (a bb pane) whose touch point (touchX, touchY) is hit-tested at full size at
  // (toX, toY).
  void moveArea(uint8_t *area, float touchX, float touchY, float toX, float toY) const;
};

// The roots the game places itself in touch pixels (cells, cursors, icons), put where a frame takes
// them. The game's own place is kept, so a root it has not moved since is not placed twice.
class FrameRoots {
public:
  void place(void *layout, const PageFrame &frame);

private:
  struct Place {
    float x;
    float y;
    float scaleX;
    float scaleY;

    bool operator==(const Place &other) const {
      return x == other.x && y == other.y && scaleX == other.scaleX && scaleY == other.scaleY;
    }
  };
  struct Entry {
    uint8_t *root;
    Place game;
    Place placed;
    uint32_t used;
  };
  // A screen's cells, cursors and icons.
  static constexpr size_t kCapacity = 64;

  static Place read(const uint8_t *root);
  // The root's entry, or the least recently used one for a root not seen before.
  Entry &find(const uint8_t *root);

  Entry entries_[kCapacity] = {};
  uint32_t clock_ = 0;
};

} // namespace z3dmods::mm3d
