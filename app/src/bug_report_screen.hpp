// The bug report a player makes from a running game (kReportKey): the game held where it was, and
// a lucent::bug_report draft of what it showed (both screens), a state that returns to it, the
// core's log and the run's facts, with the shared RmlUi form (bug_report::Form) for the player's
// words. The state is written on a worker while the form is up. A saved report is
// <user>/bug-reports/<stamp>-<summary>/ (report.json, README.md and the files); a cancelled one
// leaves nothing. The game plays on from the same frame afterwards.
#pragma once

#include "rml_host.hpp"

#include <lucent/bug_report.h>

#include <filesystem>
#include <future>
#include <optional>
#include <string>

namespace bug_report {
class Form;
} // namespace bug_report

namespace z3d::app {

class Emulator;
class GameOptions;
struct AppConfig;
class GameWindow;
class MenuCanvas;
class PadInput;
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
  // Captures into a new draft under `<config.userDir>/bug-reports`; the emulator's frame() is the
  // frame the report shows. The emulator must not step until the screen is gone.
  BugReportScreen(const MenuCanvas &canvas, GameWindow &window, const PadInput &pad,
                  Emulator &emulator, const AppConfig &config, const ReportedRun &run);
  ~BugReportScreen();
  BugReportScreen(const BugReportScreen &) = delete;
  BugReportScreen &operator=(const BugReportScreen &) = delete;
  BugReportScreen(BugReportScreen &&) = delete;
  BugReportScreen &operator=(BugReportScreen &&) = delete;

  // Until the player saves or cancels, or the window closes: the saved report's folder, if one
  // was saved.
  [[nodiscard]] std::optional<std::filesystem::path> run();

  static constexpr const char *kFolder = "bug-reports";

private:
  void capture(Emulator &emulator, const std::filesystem::path &userDir, const ReportedRun &run);
  // Waits for the worker's state and attaches it once it is written.
  void attachState(lucent::bug_report::Draft &draft);
  [[nodiscard]] std::optional<std::filesystem::path> save(lucent::bug_report::Draft &draft,
                                                          bug_report::Form &form);

  GameWindow &window_;
  const PadInput &pad_;
  RmlHost host_;
  std::optional<lucent::bug_report::Draft> draft_;
  // Whether the worker wrote the state file.
  std::future<bool> state_;
};

} // namespace z3d::app
