// The game's own options, which live in the loaded file (decomp docs/ui.md "Options"), held at the
// player's choices from the patch's block every play frame, the volume applied as the Options
// screen applies it. The 3DS-only ones are held at what the port needs: the Circle Pad Pro in use,
// as the free camera turns only with it, and motion on, which the app's mouse aim turns through.
#include "../common/guest.hpp"
#include "options.hpp"
#include "play_update.hpp"
#include "symbols.hpp"

#include <stdint.h>

namespace z3dmods::mm3d {
namespace {

using SetVolumeFn = void (*)(int32_t setting);

inline constexpr uint8_t kCirclePadProUse = 1;
inline constexpr uint8_t kMotionOn = 0;

void hold(void *common, uint32_t offset, uint8_t value) {
  if (guest::read<uint8_t>(common, offset) != value) {
    guest::write(common, offset, value);
  }
}

} // namespace

void holdCommonOptions() {
  auto *common = reinterpret_cast<void *>(kCommonData);
  const volatile Options &chosen = options();
  hold(common, kCommonTargetHold, chosen.targetHold);
  hold(common, kCommonFreeCameraInvert, chosen.freeCameraInvert);
  hold(common, kCommonFirstPersonInvert, chosen.firstPersonInvert);
  hold(common, kCommonSwimInvert, chosen.swimInvert);
  hold(common, kCommonCirclePadPro, kCirclePadProUse);
  hold(common, kCommonMotionOff, kMotionOn);
  if (guest::read<uint8_t>(common, kCommonVolume) != chosen.volume) {
    guest::write(common, kCommonVolume, chosen.volume);
    guest::function<SetVolumeFn>(kAudioSetVolumeSetting)(chosen.volume);
  }
}

} // namespace z3dmods::mm3d
