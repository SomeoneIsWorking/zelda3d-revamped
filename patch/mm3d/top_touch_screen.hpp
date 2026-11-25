// Where the title's and file select's touch screens sit on the top screen (top_touch_screen.cpp).
#pragma once

namespace z3dmods::mm3d {

// The touch screen's width and the top screen's, in pixels.
inline constexpr float kTouchWidth = 320.0F;
inline constexpr float kTopWidth = 400.0F;
// The touch screen's left edge on the top screen: in its middle.
inline constexpr float kTouchLeft = (kTopWidth - kTouchWidth) / 2.0F;

} // namespace z3dmods::mm3d
