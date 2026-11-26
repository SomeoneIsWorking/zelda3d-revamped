// OoT3D USA guest layout (code.bin md5 20c7ec62...): addresses and offsets come from decomp/oot3d.
#pragma once

#include "symbols.hpp"

#include <stdint.h>

namespace z3dmods::oot3d {

inline constexpr int32_t kGameModeNormal = 0;
inline constexpr int32_t kGameModeFileSelect = 2;
inline constexpr int32_t kFileNumDebug = 0xFEDC;

// sceneLoadFlag: 20 asks for an outgoing scene transition; -20 is the incoming fade, 0 none.
inline constexpr int8_t kTransitionStart = 20;
inline constexpr uint32_t kButtonStart = 1U << 3;

inline constexpr uint8_t kCsStateRunning = 2;

// R_UPDATE_RATE is SREG(30): the frame-rate divisor.
inline constexpr uint32_t kUpdateRate = kGameInfoData + sizeof(int16_t) * (kRegsPerGroup + 30);

// The touch screen's width and the top screen's, in pixels. While a menu is widened the touch
// screen's projection spans x -kWideMargin to 320 + kWideMargin (top_touch_screen.cpp).
inline constexpr float kTouchWidth = 320.0F;
inline constexpr float kTopWidth = 400.0F;
inline constexpr float kWideMargin = (kTopWidth - kTouchWidth) / 2;

// The One Screen 891b60e's Ocarina_Draw: its copy of the ocarina's screen on the top screen
// (decomp docs/ocarina.md).
inline constexpr uintptr_t kBaseOcarinaDraw = 0x610D88;

// TEXT_STATE_CHOICE
inline constexpr int32_t kMessageStateChoice = 4;

} // namespace z3dmods::oot3d
