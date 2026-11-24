// The interactive loop: the core runs frames at the 3DS's rate until the window is closed, the
// player asks for the title picker, the game's options or a bug report; a key or button held from
// the screen before is let go before the game reads any; the mod keys swap the running game's mods
// (mod_swap.hpp). Frame interpolation is on while it plays when the game's option asks for it, and
// the pacing (ticks, vblanks and the window's flips) and the replays' outcomes are logged every
// kPacingReport. With a record folder, each present's top screen is saved there, numbered from 0.
#pragma once

#include "frame_recorder.hpp"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <utility>

namespace z3d::app {

class Emulator;
class FrameInterpolation;
class GameOptions;
class GameWindow;
class ModSwap;
class PadInput;

inline constexpr std::chrono::seconds kPacingReport{10};

class Player {
public:
  Player(Emulator &emulator, GameWindow &window, ModSwap &mods, const GameOptions &options,
         PadInput &pad, FrameInterpolation &interpolation, std::filesystem::path record)
      : emulator_(emulator), window_(window), mods_(mods), options_(options), pad_(pad),
        interpolation_(interpolation) {
    if (!record.empty()) {
      recorder_.emplace(std::move(record));
    }
  }

  enum class Left : uint8_t { Closed, Menu, Settings, Report };
  // Until the window closes, the menu or the options are asked for, or a bug report (whose
  // frame, both screens, is then the emulator's frame()).
  [[nodiscard]] Left run();

private:
  void swapMods();

  Emulator &emulator_;
  GameWindow &window_;
  ModSwap &mods_;
  const GameOptions &options_;
  PadInput &pad_;
  FrameInterpolation &interpolation_;
  std::optional<FrameRecorder> recorder_;
};

} // namespace z3d::app
