// The pause screens our way (pause_quads.hpp). Each screen's builder is overridden: the game's
// own runs (hooks.s, the builder's first instruction and a branch back into it), then its quads
// are placed ours: BACK cornered, the tabs' bar off the screen, the tabs down the left edge, and
// on the item screen the panel and the button column off the screen for ours. Touch_InRect is ours
// too, so that while one of them is open BACK and the tabs answer where we draw them; the rest of
// it is the game's: the stylus's point inside the rect, edges in, once its flag for the phase is
// up.
#include "pause_screens.hpp"

#include "../common/guest.hpp"
#include "inventory_screen.hpp"
#include "menu_quads.hpp"
#include "pause_quads.hpp"
#include "slot_column.hpp"
#include "symbols.hpp"

namespace z3dmods::oot3d {

// The game's builders, from hooks.s.
void itemMenuGame();
void gearMenuGame();
void mapMenuGame();
void saveMenuGame();

namespace {

using SetUvsFn = void (*)(void *batch, const float *corner, const float *size, int32_t count,
                          int32_t first);

// Touch_InRect's phases, each with its flag in gTouch; a phase past them asks no flag.
inline constexpr uint32_t kPhaseFlags[] = {kTouchPhase0, kTouchPhase1, kTouchPhase2, kTouchPhase3};
// The item screen's panel's two quads and the button column's stone and its buttons' frames.
inline constexpr uint32_t kPanelQuads = 2;
inline constexpr uint32_t kColumnQuads = 13;
// BACK's two quads and BACK lit's three; the overworld map cuts lit's last into three strips.
inline constexpr uint32_t kBackQuads = 2;
inline constexpr uint32_t kBackLitQuads = 3;
inline constexpr uint32_t kMapBackLitQuads = 5;
// The Map tab's face, the item screen's middle tab.
inline constexpr uint32_t kMapTab = 1;

constinit PauseQuads itemQuads;
constinit PauseQuads gearQuads;
constinit PauseQuads mapQuads;
constinit PauseQuads otherMapQuads;

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
    drawn = itemQuads.back();
  } else if (!itemQuads.tab(asked, drawn) && !gearQuads.tab(asked, drawn) &&
             !mapQuads.tab(asked, drawn)) {
    return asked;
  }
  // The part on the touch screen; the app touches its edge for a press beside it.
  const float left = drawn.x > 0.0F ? drawn.x : 0.0F;
  const float right = drawn.x + drawn.width > 0.0F ? drawn.x + drawn.width : 0.0F;
  return {static_cast<int32_t>(left), static_cast<int32_t>(drawn.y),
          static_cast<int32_t>(right - left), static_cast<int32_t>(drawn.height)};
}

float tableValue(uintptr_t table, uint32_t quad, uint32_t axis) {
  return guest::read<float>(table + (quad * 2 + axis) * sizeof(float));
}

// The Map tab's cell a texel lower and shorter: its top row took in the Gear tab's art just above
// it in the sheet, a thin line over the tab that the tabs' bar hid.
void trimMapTabCell(uintptr_t batch) {
  const uint32_t quad = kItemMenuQuadTabs + kMapTab;
  const float corner[] = {tableValue(kItemMenuQuadUvs, quad, 0),
                          tableValue(kItemMenuQuadUvs, quad, 1) + 1.0F};
  const float size[] = {tableValue(kItemMenuQuadUvSizes, quad, 0),
                        tableValue(kItemMenuQuadUvSizes, quad, 1) - 1.0F};
  guest::function<SetUvsFn>(kQuadBatchSetUvs)(reinterpret_cast<void *>(batch), corner, size, 1,
                                              static_cast<int32_t>(quad));
}

} // namespace

void initItemMenu() {
  itemMenuGame();
  const auto batch = guest::read<uintptr_t>(kItemMenu + kItemMenuQuads);
  const QuadTables tables{kItemMenuQuadPositions, kItemMenuQuadSizes};
  itemQuads.build(batch, {tables, kItemMenuQuadTabs, kItemMenuQuadTabFrames, kItemMenuQuadTabBar,
                          kItemMenuQuadBack, kItemMenuQuadBackLit, kBackLitQuads});
  shiftQuads(batch, tables, {kItemMenuQuadPanel, kPanelQuads}, kOffScreen);
  shiftQuads(batch, tables, {kItemMenuQuadColumn, kColumnQuads}, kOffScreen);
  trimMapTabCell(batch);
  buildInventoryScreen();
  buildSlotColumn();
}

void initGearMenu() {
  gearMenuGame();
  gearQuads.build(guest::read<uintptr_t>(kGearMenu + kGearMenuQuads),
                  {{kGearMenuQuadPositions, kGearMenuQuadSizes},
                   kGearMenuQuadTabs,
                   kGearMenuQuadTabFrames,
                   kGearMenuQuadTabBar,
                   kGearMenuQuadBack,
                   kGearMenuQuadBackLit,
                   kBackLitQuads});
}

void initMapMenu() {
  mapMenuGame();
  mapQuads.build(guest::read<uintptr_t>(kMapMenu + kMapMenuQuads),
                 {{kMapMenuQuadPositions, kMapMenuQuadSizes},
                  kMapMenuQuadTabs,
                  kMapMenuQuadTabFrames,
                  kMapMenuQuadTabBar,
                  kMapMenuQuadBack,
                  kMapMenuQuadBackLit,
                  kMapBackLitQuads});
  otherMapQuads.build(guest::read<uintptr_t>(kMapMenu + kMapMenuOtherQuads),
                      {{kMapMenuOtherQuadPositions, kMapMenuOtherQuadSizes},
                       kMapMenuOtherQuadTabs,
                       kMapMenuOtherQuadTabFrames,
                       kMapMenuOtherQuadTabBar,
                       kMapMenuOtherQuadBack,
                       kMapMenuOtherQuadBackLit,
                       kBackLitQuads});
}

void initSaveMenu() {
  saveMenuGame();
  const auto batch = guest::read<uintptr_t>(kSaveMenu + kSaveMenuQuads);
  const QuadTables tables{kSaveMenuQuadPositions, kSaveMenuQuadSizes};
  shiftQuads(batch, tables, {kSaveMenuQuadBack, kBackQuads}, kCorneredBack);
  shiftQuads(batch, tables, {kSaveMenuQuadBackLit, kBackLitQuads}, kCorneredBack);
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
