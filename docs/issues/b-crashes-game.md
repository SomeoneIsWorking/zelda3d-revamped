# Pressing B crashes the game

User, 2026-11-27 (Fedora): "pressing B crashes the game".

Reported after the OoT3D hook sites became overrides (3aba98c to 3eae04b).

User, 2026-11-27 (Fedora): "bug report key, ESC also crashed when I tried to skip a cutscene, does
ESC exit or something?" B is the keyboard's bug report key; Escape crashed too, pressed to skip a
cutscene.

## Cause
Not a crash in the game: the Wayland compositor dropped the app's connection. Opening a screen
over the game (the bug report on B, the options on Escape or View) makes the menu canvas's SDL
Vulkan renderer, a second swapchain on the window's `wl_surface` while the core's still held it.
Mesa creates a `wp_fifo_v1` object per swapchain, and a second one on a surface is a protocol
error: `wp_fifo_manager_v1#95: error 0: Attempted to create a second fifo surface for the
wl_surface`, then `Wayland display connection closed by server (fatal)`. The input tests run under
Gamescope's X11 and never see it.

Reproduced with `kwin_wayland --virtual` nested in a private D-Bus session, the app a Wayland client
in it, the test pad's View opening the options.

## Fix
The screen over the game holds a `WindowLoan` (`app/src/emulator.cpp`): presenting goes off and the
core releases its swapchain (`RendererBase::ReleaseWindow`, Azahar fork b2bc2e267) before the
canvas makes its own; the game's next frame after the screen makes the core's again. In the
nested KWin the options open and close, the app lives, no fifo error is logged and the game
presents at 60 flips a second after the close.
