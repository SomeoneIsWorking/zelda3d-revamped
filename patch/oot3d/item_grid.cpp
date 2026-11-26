// The item screen's grid, placed between the tabs down the left edge (pause_screens.cpp) and the
// slot column. The game's four functions that know where the grid is are ours (hooks.s
// item_node_position, item_place_cursor, item_update_cursor, item_update_touch): the same steps
// as the game's (decomp docs/pause.md), at our grid's place, and the cursor wrapping before the
// buttons' column.
#include "item_grid.hpp"

#include "../common/guest.hpp"
#include "game.hpp"
#include "item_menu_nodes.hpp"
#include "pause_screens.hpp"
#include "symbols.hpp"

#include <stdint.h>

namespace z3dmods::oot3d {
namespace {

// The game passes floats in VFP registers; the patch is built softfp.
// NOLINTBEGIN(modernize-use-using): only a typedef takes the calling convention.
typedef void (*AimFn)(void *cursor, float x, float y) __attribute__((pcs("aapcs-vfp")));
typedef void (*FadeFn)(void *cursor, float fade) __attribute__((pcs("aapcs-vfp")));
typedef void (*SetAlphaFn)(void *layout, int32_t node, float alpha)
    __attribute__((pcs("aapcs-vfp")));
// NOLINTEND(modernize-use-using)
using PadFn = uint32_t (*)();
using SoundFn = void (*)(uint32_t sfx, const void *position, uint8_t token, const float *frequency,
                         const float *volume, const int8_t *reverb);
using SetIconFn = void (*)(void *layout, int32_t node, int32_t item);
using SetRectFn = void (*)(void *layout, int32_t node, int32_t x, int32_t y, int32_t width,
                           int32_t height);
using PlaceHeldFn = void (*)(void *layout, int32_t node, int32_t x, int32_t y, int32_t width,
                             int32_t height, int32_t offsetX, int32_t offsetY);
using AimCornersFn = void (*)(int32_t show, int32_t announce);
using ShowNodeFn = void (*)(int32_t node, int32_t chosen);
using CheckFn = int32_t (*)();
using ApplyLayoutFn = void (*)(int32_t);
using CloseFn = void (*)(int32_t open, int32_t sound);
using TouchPointFn = void (*)(uint16_t *x, uint16_t *y, uint8_t *down);

// The grid: node (column, row) at left + pitch * column, top + pitch * row, its icon 42 px.
inline constexpr float kGridLeft = 30.0F;
inline constexpr float kGridTop = 21.0F;
inline constexpr float kGridPitch = 52.0F;
inline constexpr int32_t kIconSide = 42;
// The cursor's corners sit this far up and left of the node.
inline constexpr int32_t kCornersOutside = 2;
// The held item's icon over its node: its side and how far up and left it sits.
inline constexpr int32_t kHeldSide = 52;
inline constexpr int32_t kHeldOffset = -5;
// The game's places of the button nodes, which the slot column replaces: their node, where
// ItemMenu_NodePosition puts them and where ItemMenu_PlaceCursor aims the corners at them.
struct ButtonNode {
  int32_t node;
  float x;
  float y;
  float cornersX;
  float cornersY;
};
inline constexpr ButtonNode kButtonNodes[] = {{5, 271.0F, 7.0F, 269.0F, 5.0F},
                                              {11, 275.0F, 73.0F, 273.0F, 71.0F},
                                              {17, 263.0F, 125.0F, 261.0F, 123.0F},
                                              {23, 271.0F, 191.0F, 269.0F, 189.0F}};
// The buttons X and Y assign to: the nodes 11 and 17.
inline constexpr int32_t kNodeX = 11;
inline constexpr int32_t kNodeY = 17;
// Where corners go while they aim at nothing.
inline constexpr float kCornersOffX = 400.0F;
inline constexpr float kCornersOffY = 240.0F;
inline constexpr int32_t kNone = -1;
inline constexpr float kHidden = 0.0F;
inline constexpr float kShown = 1.0F;
inline constexpr float kCursorFadeOut = -1.0F;

// gItemMenuState's values the cursor moves in and goes to.
enum State : uint8_t {
  kStateChoosing = 1,
  kStateSelecting = 2,
  kStateMoving = 8,
  kStateMoved = 9,
  kStateBowSwap = 12,
  kStateClosing = 14,
};

struct Place {
  int32_t x;
  int32_t y;
};

void *menuAddress() {
  return reinterpret_cast<void *>(static_cast<uintptr_t>(kItemMenu));
}

template <typename T> T field(uint32_t offset) {
  return guest::read<T>(menuAddress(), offset);
}

template <typename T> void setField(uint32_t offset, T value) {
  guest::write(menuAddress(), offset, value);
}

int32_t state() {
  return guest::read<int32_t>(kItemMenuState);
}

void setState(State state) {
  *reinterpret_cast<volatile int32_t *>(static_cast<uintptr_t>(kItemMenuState)) =
      static_cast<int32_t>(state);
}

void *layoutAt(uint32_t offset) {
  return field<void *>(offset);
}

Place gridPlace(int32_t column, int32_t row) {
  return {static_cast<int32_t>(kGridLeft + kGridPitch * static_cast<float>(column)),
          static_cast<int32_t>(kGridTop + kGridPitch * static_cast<float>(row))};
}

Place nodePlace(int32_t node) {
  float x = 0.0F;
  float y = 0.0F;
  itemNodePosition(node, &x, &y);
  return {static_cast<int32_t>(x), static_cast<int32_t>(y)};
}

int32_t cursorNode() {
  return field<int32_t>(kItemMenuCursorColumn) +
         field<int32_t>(kItemMenuCursorRow) * kItemMenuColumns;
}

uint8_t nodeSlot(int32_t node) {
  return itemMenuNodes(linkIsChild())[node];
}

void play(uint32_t sfx) {
  const auto *scale =
      reinterpret_cast<const float *>(static_cast<uintptr_t>(kSfxDefaultFreqAndVolScale));
  const auto *reverb = reinterpret_cast<const int8_t *>(static_cast<uintptr_t>(kSfxDefaultReverb));
  guest::function<SoundFn>(kAudioPlaySoundGeneral)(sfx, nullptr, 4, scale, scale, reverb);
}

void aim(uint32_t cursorOffset, float x, float y) {
  guest::function<AimFn>(kMenuCursorAim)(layoutAt(cursorOffset), x, y);
}

void hideTarget() {
  setField(kItemMenuSavedColumn, kNone);
  setField(kItemMenuSavedRow, kNone);
  aim(kItemMenuTargetCursor, kCornersOffX, kCornersOffY);
}

void heldAlpha(float alpha) {
  guest::function<SetAlphaFn>(kHudItemButtonsSetAlpha)(layoutAt(kItemMenuHeld), 0, alpha);
}

void fadeCursor() {
  guest::function<FadeFn>(kMenuCursorFade)(layoutAt(kItemMenuCursor), kCursorFadeOut);
}

void aimCorners(bool announce) {
  guest::function<AimCornersFn>(kItemMenuAimCorners)(1, announce ? 1 : 0);
}

void setFrom(int32_t node) {
  const Place place = nodePlace(node);
  setField(kItemMenuFromNode, node);
  setField(kItemMenuFromX, static_cast<uint16_t>(place.x));
  setField(kItemMenuFromY, static_cast<uint16_t>(place.y));
}

void setTo(int32_t node) {
  const Place place = nodePlace(node);
  setField(kItemMenuToNode, node);
  setField(kItemMenuToX, static_cast<uint16_t>(place.x));
  setField(kItemMenuToY, static_cast<uint16_t>(place.y));
}

void restartSwap() {
  setField(kItemMenuSwapDelay, 0);
  setField(kItemMenuSwapTimer, 0);
}

// The move's line from its node to its target, both copies.
void lineSwap() {
  const uint16_t line[] = {field<uint16_t>(kItemMenuFromX), field<uint16_t>(kItemMenuFromY),
                           field<uint16_t>(kItemMenuToX), field<uint16_t>(kItemMenuToY)};
  for (uint32_t index = 0; index < 4; ++index) {
    setField(kItemMenuSwapLine + index * 2, line[index]);
    setField(kItemMenuSwapStart + index * 2, line[index]);
  }
}

// The moved item's icon shown over its node.
void holdFromItem() {
  heldAlpha(kShown);
  const uint8_t slot = nodeSlot(field<int32_t>(kItemMenuFromNode));
  guest::function<SetIconFn>(kHudItemButtonsSetIcon)(layoutAt(kItemMenuHeld), 0,
                                                     inventoryItem(slot));
  guest::function<PlaceHeldFn>(kItemMenuPlaceHeld)(
      layoutAt(kItemMenuHeld), 0, field<uint16_t>(kItemMenuFromX), field<uint16_t>(kItemMenuFromY),
      kHeldSide, kHeldSide, kHeldOffset, kHeldOffset);
}

// The move turned down: back to choosing a node.
void refuse() {
  heldAlpha(kHidden);
  fadeCursor();
  guest::function<ApplyLayoutFn>(kItemMenuApplyLayout)(1);
}

// The moved item back on its node, the move dropped.
void dropBack(bool announce) {
  guest::function<SetRectFn>(kHudItemButtonsSetRect2D)(
      layoutAt(kItemMenuItems), field<int32_t>(kItemMenuFromNode), field<uint16_t>(kItemMenuFromX),
      field<uint16_t>(kItemMenuFromY), kIconSide, kIconSide);
  hideTarget();
  setState(kStateSelecting);
  play(kSfxMenuCancel);
  aimCorners(announce);
  heldAlpha(kHidden);
  fadeCursor();
}

void toBowSwap() {
  setState(kStateBowSwap);
  setField(kItemMenuStateStep, 1);
}

// The pad's moves of the cursor, wrapping before the buttons' column; each with its sound while
// selecting. `fromLeft` skips the move right, as the game does after X or Y.
void moveCursor(bool sounds, bool fromLeft) {
  const auto repeated = guest::function<PadFn>(kPadRepeated);
  int32_t column = field<int32_t>(kItemMenuCursorColumn);
  int32_t row = field<int32_t>(kItemMenuCursorRow);
  int32_t moves = 0;
  const auto step = [&](uint32_t button) {
    if ((repeated() & button) == 0) {
      return false;
    }
    if (sounds) {
      play(kSfxMenuCursor);
    }
    ++moves;
    return true;
  };
  if (!fromLeft && step(kButtonRight)) {
    column = column + 1 < kGridColumns ? column + 1 : 0;
  }
  if (step(kButtonLeft)) {
    column = column > 0 ? column - 1 : kGridColumns - 1;
  }
  if (step(kButtonUp)) {
    row = row > 0 ? row - 1 : kItemMenuRows - 1;
  }
  if (step(kButtonDown)) {
    row = row + 1 < kItemMenuRows ? row + 1 : 0;
  }
  setField(kItemMenuCursorColumn, column);
  setField(kItemMenuCursorRow, row);
  if (moves == 0) {
    return;
  }
  play(kSfxMenuCursor);
  aimCorners(true);
  setField(kItemMenuCursorTimer, 0);
}

// Selecting: A picks the cursor's item up, X and Y assign it, B closes the screen.
void select() {
  const auto pressed = guest::function<PadFn>(kPadPressed)();
  if ((pressed & (kButtonX | kButtonY)) != 0) {
    const int32_t node = cursorNode();
    if (nodeSlot(node) == kSlotNone) {
      return;
    }
    setFrom(node);
    restartSwap();
    setTo((pressed & kButtonX) != 0 ? kNodeX : kNodeY);
    if (guest::function<CheckFn>(kItemMenuSameBowSwap)() != 0) {
      toBowSwap();
      return;
    }
    lineSwap();
    if (guest::function<CheckFn>(kItemMenuCanPlace)() == 0) {
      refuse();
      setState(kStateChoosing);
      return;
    }
    if (nodeSlot(node) != kSlotNone) {
      holdFromItem();
    }
    play(kSfxMenuDecide);
    setField(kItemMenuChosenNode, node);
    setState(kStateMoved);
    moveCursor(true, true);
    return;
  }
  if ((pressed & kButtonB) != 0) {
    guest::function<CloseFn>(kPauseMenuClose)(0, 1);
    setField(kItemMenuCloseTimer, 0);
    setState(kStateClosing);
    return;
  }
  if ((pressed & kButtonA) != 0) {
    const int32_t node = cursorNode();
    if (nodeSlot(node) == kSlotNone) {
      return;
    }
    play(kSfxMenuDecide);
    setFrom(node);
    setField(kItemMenuSavedRow, field<int32_t>(kItemMenuCursorRow));
    setField(kItemMenuSavedColumn, field<int32_t>(kItemMenuCursorColumn));
    aim(kItemMenuTargetCursor,
        static_cast<float>(field<uint16_t>(kItemMenuFromX) - kCornersOutside),
        static_cast<float>(field<uint16_t>(kItemMenuFromY) - kCornersOutside));
    restartSwap();
    holdFromItem();
    setState(kStateMoving);
    return;
  }
  moveCursor(true, false);
}

// Moving a picked item: A drops it on the cursor's node, X and Y assign it, B puts it back.
void move() {
  const auto pressed = guest::function<PadFn>(kPadPressed)();
  if ((pressed & kButtonA) != 0) {
    setTo(cursorNode());
    if (guest::function<CheckFn>(kItemMenuSameBowSwap)() != 0) {
      heldAlpha(kHidden);
      toBowSwap();
      return;
    }
    if (field<int32_t>(kItemMenuToNode) == field<int32_t>(kItemMenuFromNode)) {
      dropBack(false);
      return;
    }
    lineSwap();
    hideTarget();
    restartSwap();
    if (guest::function<CheckFn>(kItemMenuCanPlace)() == 0) {
      refuse();
      setState(kStateChoosing);
      return;
    }
    setState(kStateMoved);
    play(kSfxMenuDecide);
    return;
  }
  if ((pressed & (kButtonX | kButtonY)) != 0) {
    setTo((pressed & kButtonX) != 0 ? kNodeX : kNodeY);
    restartSwap();
    lineSwap();
    const bool placed = guest::function<CheckFn>(kItemMenuCanPlace)() != 0;
    if (!placed) {
      refuse();
    } else {
      if (nodeSlot(field<int32_t>(kItemMenuFromNode)) != kSlotNone) {
        holdFromItem();
      }
      play(kSfxMenuDecide);
      setField(kItemMenuChosenNode, field<int32_t>(kItemMenuFromNode));
    }
    setField(kItemMenuCursorColumn, field<int32_t>(kItemMenuSavedColumn));
    setField(kItemMenuCursorRow, field<int32_t>(kItemMenuSavedRow));
    aimCorners(true);
    hideTarget();
    setState(placed ? kStateMoved : kStateChoosing);
    if (placed) {
      moveCursor(false, false);
    }
    return;
  }
  if ((pressed & kButtonB) != 0) {
    dropBack(true);
    return;
  }
  moveCursor(false, false);
}

// The stylus on node `node` at `place`, its cursor column and row.
struct Target {
  int32_t node;
  Place place;
  int32_t column;
  int32_t row;
  float cornersX;
  float cornersY;
};

// The stylus went down on `target`: an empty node announced, an item pressed.
void press(const Target &target, int phase) {
  const auto show = guest::function<ShowNodeFn>(kItemMenuShowNode);
  const bool empty = nodeSlot(target.node) == kSlotNone && phase == 1;
  if (!empty) {
    show(target.node, 1);
    setField(kItemMenuPressedNode, target.node);
  }
  setField(kItemMenuCursorColumn, target.column);
  setField(kItemMenuCursorRow, target.row);
  setField(kItemMenuCornersColumn, target.column);
  setField(kItemMenuCornersRow, target.row);
  if (empty) {
    if (guest::read<uint16_t>(kTouch + kTouchPhase1) != 0) {
      play(kSfxMenuCursor);
    }
    show(target.node, 0);
  }
}

// The stylus held on the pressed `target`: lifted, it picks the item up; dragged, it aims there.
void hold(const Target &target, int phase) {
  if (phase == 1) {
    setField(kItemMenuFromX, static_cast<uint16_t>(target.place.x));
    setField(kItemMenuFromY, static_cast<uint16_t>(target.place.y));
    setField(kItemMenuFromNode, target.node);
    setField(kItemMenuSavedColumn, field<int32_t>(kItemMenuCornersColumn));
    setField(kItemMenuSavedRow, field<int32_t>(kItemMenuCornersRow));
    aim(kItemMenuTargetCursor, target.cornersX, target.cornersY);
    setField(kItemMenuCornersColumn, kNone);
    setField(kItemMenuCornersRow, kNone);
  } else if (phase == 2) {
    setField(kItemMenuToColumn, static_cast<uint16_t>(target.column));
    setField(kItemMenuToRow, static_cast<uint16_t>(target.row));
    setField(kItemMenuToX, static_cast<uint16_t>(target.place.x));
    setField(kItemMenuToY, static_cast<uint16_t>(target.place.y));
    setField(kItemMenuToNode, target.node);
  }
}

// Whether the stylus took `target`: 0 not, 1 pressed (the game's 0), 2 held (the game's 1).
int touch(const Target &target, int phase) {
  const int32_t pressed = field<int32_t>(kItemMenuPressedNode);
  const int touchPhase = pressed == kNone ? 0 : 2;
  if (touchInRect(target.place.x, target.place.y, kIconSide, kIconSide, touchPhase) == 0) {
    return 0;
  }
  if (pressed == kNone) {
    press(target, phase);
    return 1;
  }
  if (pressed != target.node) {
    return 0;
  }
  hold(target, phase);
  return 2;
}

} // namespace

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters): ItemMenu_NodePosition's own arguments.
void itemNodePosition(int node, float *x, float *y) {
  for (const ButtonNode &button : kButtonNodes) {
    if (button.node == node) {
      *x = button.x;
      *y = button.y;
      return;
    }
  }
  const int column = node % kItemMenuColumns;
  const int row = node / kItemMenuColumns;
  *x = kGridLeft + kGridPitch * static_cast<float>(column);
  *y = kGridTop + kGridPitch * static_cast<float>(row);
}

void placeItemCursor() {
  const int32_t node = cursorNode();
  setField(kItemMenuAimedNode, node);
  const auto column = field<int32_t>(kItemMenuCornersColumn);
  if (column == kNone) {
    aim(kItemMenuCursor, kCornersOffX, kCornersOffY);
    return;
  }
  for (const ButtonNode &button : kButtonNodes) {
    if (button.node == node) {
      aim(kItemMenuCursor, button.cornersX, button.cornersY);
      return;
    }
  }
  const Place place = gridPlace(column, field<int32_t>(kItemMenuCornersRow));
  aim(kItemMenuCursor, static_cast<float>(place.x - kCornersOutside),
      static_cast<float>(place.y - kCornersOutside));
}

void updateItemCursor() {
  const int32_t now = state();
  if (now == kStateSelecting) {
    select();
  } else if (now == kStateMoving) {
    move();
  }
}

int updateItemTouch(int phase) {
  for (int32_t row = 0; row < kItemMenuRows; ++row) {
    for (int32_t column = 0; column < kGridColumns; ++column) {
      const int32_t node = column + row * kItemMenuColumns;
      const Place place = nodePlace(node);
      const int took =
          touch({node, place, column, row, static_cast<float>(place.x - kCornersOutside),
                 static_cast<float>(place.y - kCornersOutside)},
                phase);
      if (took != 0) {
        return took == 2 ? 1 : 0;
      }
    }
  }
  for (int32_t row = 0; row < kItemMenuRows; ++row) {
    const ButtonNode &button = kButtonNodes[row];
    const int took = touch({button.node,
                            {static_cast<int32_t>(button.x), static_cast<int32_t>(button.y)},
                            kGridColumns,
                            row,
                            button.cornersX,
                            button.cornersY},
                           phase);
    if (took != 0) {
      return took == 2 ? 1 : 0;
    }
  }
  uint16_t x = 0;
  uint16_t y = 0;
  uint8_t down = 0;
  guest::function<TouchPointFn>(kTouchPointGet)(&x, &y, &down);
  if (down == 0) {
    setField(kItemMenuPressedNode, kNone);
  }
  return 0;
}

} // namespace z3dmods::oot3d
