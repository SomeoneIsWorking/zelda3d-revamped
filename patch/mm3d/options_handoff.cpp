// File select's Options screen hands over to the app's options, whose categories hold the game's
// own (common_options.cpp) without its 3DS-only ones: on the screen's first update the patch asks
// for them through the link and the app opens them over the game; on the next it leaves through
// the game's own commit, back to file select (decomp docs/ui.md "Options").
#include "../common/guest.hpp"
#include "slot_link.hpp"
#include "symbols.hpp"

#include <stdint.h>

namespace z3dmods::mm3d {
namespace {

using CommitFn = void (*)(void *screen, void *context);

class OptionsHandoff {
public:
  void update(void *screen, void *context) {
    if (!asked_) {
      volatile SlotLink &link = slotLink();
      link.options = static_cast<uint8_t>(link.options + 1);
      asked_ = true;
      return;
    }
    // The commit changes screens only while the context lets it; it is run again until it does.
    guest::function<CommitFn>(kOptionsScreenCommit)(screen, context);
    asked_ = guest::read<void *>(context, kUiNextScreen) == nullptr;
  }

private:
  bool asked_ = false;
};

constinit OptionsHandoff handoff;

} // namespace

// OptionsScreen's vtable entry for OptionsScreen_Update (hooks.s).
void updateOptions(void *screen, void *context) {
  handoff.update(screen, context);
}

} // namespace z3dmods::mm3d
