# Every platform, every milestone

User: "a GitHub release on every milestone, all possible platforms, macOS (.app), Linux
(.AppImage), Windows (.exe), Android (.apk)", "touch controls for Android when not using a
controller (hotswap)". The rules are the `release` and `android` skills.

## Where each platform stands
- Linux: released (`z3d-x86_64.AppImage`, `z3dmods package`, `.github/workflows/release.yml`).
- macOS: builds and plays from a checkout on Apple silicon (AGENTS.md); `z3dmods shortcut` makes a
  local `~/Applications/z3d.app` that runs the checkout. No self-contained `.app` yet.
- Windows: never built. The app is SDL3 and Vulkan, which Windows has; the core builds on Windows
  upstream.
- Android: never built. Azahar's core runs on Android upstream; our frontend is SDL3, which has an
  Android backend. The setup runs as Python beside the app (plans/packed-app.md), which Android
  cannot carry, so the install needs a native path there.

## Slices
1. macOS `.app`: built (`tools/z3dmods/package_mac.py`, `z3dmods package` on a Mac): the package
   folder in `Contents/Resources`, Homebrew's libraries, the core and the Vulkan loader with
   MoltenVK's ICD in its `lib/` named `@rpath/<file>`, ad-hoc signed, `z3d-macos-arm64.zip`.
   Shown on the Mac with no Homebrew on PATH: no Homebrew mapping in the running app, OoT3D's set
   made from the dump, OoT3D at 59.8 presents and 29.9 ticks a second. The Mac session builds it.
2. Windows `.exe`: cross-built on Linux (`z3dmods package --windows`, `tools/z3dmods/package_windows.py`)
   with llvm-mingw and the libraries from `shared/target-platform`; never on hosted CI (release
   skill). `z3d/` holds the launcher `z3d.exe` (`app/launcher/windows_launcher.cpp`) and
   `bin/z3d.exe`, which links the core, SDL3, SDL3_ttf, SDL3_image, FreeType and the C++ runtime
   statically and imports only DLLs Windows ships (target-platform's `audit-windows`), beside uv's
   Windows CPython with Windows wheels; zipped as `z3d-windows-x86_64.zip`. On Windows the core is
   one static archive (`AZAHAR_CORE_STATIC`, the fork's `core_package_static.cmake`): a DLL holds
   at most 65535 exports and the core has 92208. The app ends on the console's events
   (`term_signal_windows.cpp`) and hands paths to SDL and the core as UTF-8 (`lucent::text::utf8`).
3. Android `.apk`: an SDL3 Android activity around the same app, the core's Android build, Vulkan;
   the setup run natively (the install's rules in C++ only where Python cannot go, one owner each),
   and the touch layer: the 3DS buttons and sticks drawn over the game, multi-touch, hidden while a
   controller is in use and shown again on the next touch.
4. The release: one tag builds all four and attaches them; the release notes name each file.
