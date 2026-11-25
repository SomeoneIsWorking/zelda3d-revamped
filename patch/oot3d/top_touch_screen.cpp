// The touch screen's menus on the top screen: The One Screen draws them there through a viewport of
// the top screen's middle 320 pixels, so nothing can reach its corners. While file select or a
// pause screen is up its viewport takes the whole top screen (hooks.s base_file_select_*,
// base_touch_viewport) and the touch screen's projection spans x -40 to 360 to match: everything
// stays where it was, the backdrop is spread over the width, and what is moved past the touch
// screen's edges (BACK, hooks.s back_*, <screen>_back*) shows (decomp docs/file_select.md,
// docs/pause.md).
#include "../common/guest.hpp"
#include "game.hpp"
#include "symbols.hpp"

#include <stdint.h>

namespace z3dmods::oot3d {
namespace {

using SetMatricesFn = void (*)(void *camera, float *projection, const float *view);
using ViewportFn = void (*)(int x, int y, int width, int height);
using DrawFn = void (*)(void *model);

// The touch screen's width and the top screen's, in pixels.
inline constexpr float kTouchWidth = 320.0F;
inline constexpr float kTopWidth = 400.0F;
// The projection's row for clip y, of a 4x4 row-major matrix; x is the touch screen's across.
inline constexpr int kClipYScale = 4;
inline constexpr int kClipYOffset = 7;
// The top screen's rows The One Screen gives a touch screen it draws there.
inline constexpr int kTouchFirstRow = 40;
inline constexpr int kTouchRows = 320;
// A model's draw, its vtable's fourth entry; its transform's x scale and x offset.
inline constexpr uint32_t kDrawEntry = 0xC;
inline constexpr uint32_t kScaleX = kMenuBackdropTransform;
inline constexpr uint32_t kOffsetX = kMenuBackdropTransform + 3 * sizeof(float);

bool fileSelectUp() {
  return guest::read<int32_t>(kSaveContext + kSaveGameMode) == kGameModeFileSelect &&
         guest::read<uint32_t>(kFileChoose + kFileChooseActive) != 0;
}

bool pauseUp() {
  return guest::read<int32_t>(kItemMenuState) != kMenuScreenClosed ||
         guest::read<int32_t>(kGearMenuState) != kMenuScreenClosed ||
         guest::read<int32_t>(kMapMenuState) != kMenuScreenClosed ||
         guest::read<int32_t>(kSaveMenuState) != kMenuScreenClosed;
}

bool widened() {
  return fileSelectUp() || pauseUp();
}

} // namespace

// The touch screen layers' calls of Camera2D_SetMatrices (TouchScreen2D_BeginLayer0 to 3).
void touchProjection(void *camera, float *projection, const float *view) {
  if (widened()) {
    // Clip y of x is the game's of x' = (x + margin) * 320 / 400, so x -40 to 360 spans what
    // 0 to 320 did.
    constexpr float squeeze = kTouchWidth / kTopWidth;
    constexpr float margin = (kTopWidth - kTouchWidth) / 2;
    projection[kClipYOffset] += projection[kClipYScale] * margin * squeeze;
    projection[kClipYScale] *= squeeze;
  }
  guest::function<SetMatricesFn>(kCamera2DSetMatrices)(camera, projection, view);
}

// The viewport a file select fade is drawn through: The One Screen spreads the fade's touch-screen
// quad over the top screen's rows with the game's projection, which touchProjection squeezes by
// 320 / 400, so the viewport grows by 400 / 320 about its middle.
void touchFadeViewport(int x, int y, int width, int height) {
  if (widened()) {
    const int grown = static_cast<int>(static_cast<float>(height) * kTopWidth / kTouchWidth);
    y -= (grown - height) / 2;
    height = grown;
  }
  guest::function<ViewportFn>(kGlViewport)(x, y, width, height);
}

// The viewport Presentation_Target binds: rows 40 to 360 for a touch screen it draws on the top
// screen, all 400 for a widened one.
void touchViewport(int x, int y, int width, int height) {
  if (y == kTouchFirstRow && height == kTouchRows && widened()) {
    y = 0;
    height = static_cast<int>(kTopWidth);
  }
  guest::function<ViewportFn>(kGlViewport)(x, y, width, height);
}

// The touch screen's backdrop model drawn; while widened, over x -40 to 360 instead of 0 to 320.
void drawBackdrop(void *model) {
  const auto vtable = guest::read<uintptr_t>(model, 0);
  const auto draw = guest::read<DrawFn>(vtable + kDrawEntry);
  if (!widened()) {
    draw(model);
    return;
  }
  const auto scale = guest::read<float>(model, kScaleX);
  const auto offset = guest::read<float>(model, kOffsetX);
  guest::write(model, kScaleX, scale * kTopWidth / kTouchWidth);
  guest::write(model, kOffsetX, offset - (kTopWidth - kTouchWidth) / 2);
  draw(model);
  guest::write(model, kScaleX, scale);
  guest::write(model, kOffsetX, offset);
}

} // namespace z3dmods::oot3d
