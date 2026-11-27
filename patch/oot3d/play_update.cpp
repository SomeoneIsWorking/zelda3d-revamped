// Play_Update, overridden: our per-frame features in front of the game's.
#include "play_update.hpp"

namespace z3dmods::oot3d {

// The game's Play_Update, from hooks.s.
void playGame(void *play);

namespace {

class PlayUpdate {
public:
  void update(void *play) {
    updateSlotItems(play);
    updateAutosave(play);
    updating_ = true;
    playGame(play);
    updating_ = false;
  }

  bool updating() const {
    return updating_;
  }

private:
  bool updating_ = false;
};

constinit PlayUpdate playUpdate;

} // namespace

void updatePlay(void *play) {
  playUpdate.update(play);
}

bool playUpdating() {
  return playUpdate.updating();
}

} // namespace z3dmods::oot3d
