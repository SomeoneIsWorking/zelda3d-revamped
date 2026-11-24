// The bug report a player makes from a running game (kReportKey): the game held where it was, and
// a lucent::bug_report draft of what it showed (both screens), a state that returns to it, the
// core's log and the run's facts, with a form on the menu canvas for the player's words. A saved
// report is <user>/bug-reports/<stamp>-<summary>/ (report.json, README.md and the files); a
// cancelled one leaves nothing. The game plays on from the same frame afterwards.
#pragma once

#include "menu_canvas.hpp"

#include <lucent/bug_report.h>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

struct SDL_Texture;
union SDL_Event;

namespace z3d::app {

class Emulator;
class GameOptions;
class GameWindow;
struct GameSetup;

// What a report says about the run beside what the emulator holds.
struct ReportedRun {
  std::string title;
  const GameSetup &game;
  const GameOptions &options;
  bool patchOn = true;
  bool texturesOn = true;
};

class BugReportScreen {
public:
  // Captures into a new draft under `<userDir>/bug-reports`; the emulator's frame() is the frame
  // the report shows.
  BugReportScreen(const MenuCanvas &canvas, GameWindow &window, const PadInput &pad,
                  Emulator &emulator, const std::filesystem::path &userDir, const ReportedRun &run);
  ~BugReportScreen();
  BugReportScreen(const BugReportScreen &) = delete;
  BugReportScreen &operator=(const BugReportScreen &) = delete;
  BugReportScreen(BugReportScreen &&) = delete;
  BugReportScreen &operator=(BugReportScreen &&) = delete;

  // Until the player saves (then until they read where it went) or cancels, or the window closes:
  // the saved report's folder, if one was saved.
  [[nodiscard]] std::optional<std::filesystem::path> run();

  static constexpr const char *kFolder = "bug-reports";

private:
  enum class Field : uint8_t { Summary, Description };

  void capture(Emulator &emulator, const std::filesystem::path &userDir, const ReportedRun &run);
  // What the form does with one event; true once the player asked to save.
  [[nodiscard]] bool edit(const SDL_Event &event);
  void save();
  void draw();
  // A line of text from the top left of `place`, place.h tall, wrapped at place.w when it is not
  // 0; its height.
  float text(const std::string &line, Rgb color, SDL_FRect place) const;
  [[nodiscard]] std::string &field();

  const MenuCanvas &canvas_;
  GameWindow &window_;
  const PadInput &pad_;
  std::optional<lucent::bug_report::Draft> draft_;
  SDL_Texture *frame_ = nullptr;
  float frameAspect_ = 1.0F;
  Field field_ = Field::Summary;
  lucent::bug_report::PlayerText words_;
  // Why the draft could not be made or saved, shown to the player.
  std::string problem_;
  std::optional<std::filesystem::path> saved_;
  bool cancelled_ = false;
};

} // namespace z3d::app
