// The gear and map screens: their tabs down the left edge (pause_tabs.hpp), turned once the pause
// menus build them, and their hit tests of BACK and the tabs asked there. The item screen's are
// inventory_screen.cpp's.
#include "pause_screens.hpp"

#include "../common/guest.hpp"
#include "pause_tabs.hpp"
#include "symbols.hpp"

namespace z3dmods::oot3d {
namespace {

using InitFn = void (*)();

constinit PauseTabs gearTabs;
constinit PauseTabs mapTabs;
constinit PauseTabs otherMapTabs;

} // namespace

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

int gearTouched(int x, int y, int width, int height, int phase) {
  return gearTabs.touched({x, y, width, height}, phase);
}

// Both layouts draw BACK and the tabs at the same places.
int mapTouched(int x, int y, int width, int height, int phase) {
  return mapTabs.touched({x, y, width, height}, phase);
}

} // namespace z3dmods::oot3d
