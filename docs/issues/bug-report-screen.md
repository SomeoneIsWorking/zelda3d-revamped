# The bug report screen is not Lucent's RmlUi one, and Backspace closes it

User, 2026-11-27 (Fedora): "Jesus awful, please use lucent RmlUI bug report, also backspace closes
the bug report screen"

The in-game report (B) is drawn by the app's own `BugReportScreen` on the menu canvas; it is to be
Lucent's RmlUi bug report. Backspace, meant to delete typed text, leaves the screen.

User, 2026-11-27 (Fedora): "The existing bug report also takes a while to open"

## Cause
- The screen was the app's own `BugReportScreen` drawn with SDL_ttf on the menu canvas, not the
  shared RmlUi form (`SomeoneIsWorking/bug-report`) on `lucent::bug_report`.
- Backspace: `pad_input.cpp`'s menu keys made Backspace the menus' B, and the screen cancelled on
  `PadInput::menu().b`, which reads the keyboard too.
- The wait: from the key to the form, 1.4 s on a quiet host (10.8 s while a package build loaded
  it): the savestate 1.22 s, the PNG 0.17 s, the canvas 0.07 s; the form waited for all of it.

## Fix
`BugReportScreen` shows the shared form through `RmlHost` (RmlUi's own SDL backends on the menu
canvas's renderer). The keyboard goes to the form alone; the pads' Start and B come from
`PadInput::padMenu`, the pads without the keyboard. The state is serialized on a worker while
the form is up (`Emulator::settle` on the window's thread, `serializeState` on the worker) and
attached at save. The key to the form is now 0.23 s. In the form, Return's newline is typed
only when no element took the key (`RmlHost::feed`), so Ctrl+Enter saves without one. The form's
pictures are 36% of the window's height (bug-report 9ccbdb7), so Save shows on a 16:9 window.
The input test `report` types a letter and takes it back with Backspace.
