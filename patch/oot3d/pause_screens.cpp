// The pause screens' tabs (pause_tabs.hpp): the item, gear and map screens', turned once each is
// built, and the stylus's hit test, Touch_InRect, ours so that while one of them is open BACK and
// the tabs answer where we draw them. The rest of it is the game's: the stylus's point inside the
// rect, edges in, once its flag for the phase is up.
#include "pause_screens.hpp"

#include "../common/guest.hpp"
#include "pause_tabs.hpp"
#include "symbols.hpp"

namespace z3dmods::oot3d {
namespace {

using InitFn = void (*)();

// Touch_InRect's phases, each with its flag in gTouch; a phase past them asks no flag.
inline constexpr uint32_t kPhaseFlags[] = {kTouchPhase0, kTouchPhase1, kTouchPhase2, kTouchPhase3};

constinit PauseTabs itemTabs;
constinit PauseTabs gearTabs;
constinit PauseTabs mapTabs;
constinit PauseTabs otherMapTabs;

bool pauseScreenOpen() {
  return guest::read<int32_t>(kItemMenuState) != kMenuScreenClosed ||
         guest::read<int32_t>(kGearMenuState) != kMenuScreenClosed ||
         guest::read<int32_t>(kMapMenuState) != kMenuScreenClosed;
}

// Where an open pause screen draws what `asked` hit-tests, or `asked` itself. Both map layouts
// and every screen's BACK are at the same places.
Rect drawnRect(const Rect &asked) {
  if (!pauseScreenOpen()) {
    return asked;
  }
  Box drawn{};
  if (asked.x == kPauseBackTouchX && asked.width == kPauseBackTouchWidth &&
      asked.height == kPauseBackTouchHeight) {
    drawn = itemTabs.back();
  } else if (!itemTabs.tab(asked, drawn) && !gearTabs.tab(asked, drawn) &&
             !mapTabs.tab(asked, drawn)) {
    return asked;
  }
  // The part on the touch screen; the app touches its edge for a press beside it.
  const float left = drawn.x > 0.0F ? drawn.x : 0.0F;
  const float right = drawn.x + drawn.width > 0.0F ? drawn.x + drawn.width : 0.0F;
  return {static_cast<int32_t>(left), static_cast<int32_t>(drawn.y),
          static_cast<int32_t>(right - left), static_cast<int32_t>(drawn.height)};
}

} // namespace

void turnItemTabs(uintptr_t batch) {
  itemTabs.turn(batch, {kItemMenuQuadPositions, kItemMenuQuadSizes, kItemMenuQuadTabs,
                        kItemMenuQuadTabFrames, kItemMenuQuadBack});
}

void initGearMenu() {
  guest::function<InitFn>(kGearMenuInit)();
  gearTabs.turn(guest::read<uintptr_t>(kGearMenu + kGearMenuQuads),
                {kGearMenuQuadPositions, kGearMenuQuadSizes, kGearMenuQuadTabs,
                 kGearMenuQuadTabFrames, kGearMenuQuadBack});
}

void initMapMenu() {
  guest::function<InitFn>(kMapMenuInit)();
  mapTabs.turn(guest::read<uintptr_t>(kMapMenu + kMapMenuQuads),
               {kMapMenuQuadPositions, kMapMenuQuadSizes, kMapMenuQuadTabs, kMapMenuQuadTabFrames,
                kMapMenuQuadBack});
  otherMapTabs.turn(guest::read<uintptr_t>(kMapMenu + kMapMenuOtherQuads),
                    {kMapMenuOtherQuadPositions, kMapMenuOtherQuadSizes, kMapMenuOtherQuadTabs,
                     kMapMenuOtherQuadTabFrames, kMapMenuOtherQuadBack});
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters): Touch_InRect's own arguments.
int touchInRect(int x, int y, int width, int height, int phase) {
  if (phase >= 0 && phase < static_cast<int>(sizeof(kPhaseFlags) / sizeof(kPhaseFlags[0])) &&
      guest::read<uint16_t>(kTouch + kPhaseFlags[phase]) == 0) {
    return 0;
  }
  const Rect rect = drawnRect({x, y, width, height});
  const auto left = static_cast<uint32_t>(rect.x);
  const auto top = static_cast<uint32_t>(rect.y);
  const uint32_t touchX = guest::read<uint16_t>(kTouch + kTouchX);
  const uint32_t touchY = guest::read<uint16_t>(kTouch + kTouchY);
  return static_cast<int>(touchX >= left && left + static_cast<uint32_t>(rect.width) >= touchX &&
                          touchY >= top && top + static_cast<uint32_t>(rect.height) >= touchY);
}

} // namespace z3dmods::oot3d
