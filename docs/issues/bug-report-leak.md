# Opening and closing the bug report makes the game lag more each time

User, 2026-11-27 (Fedora, 3840x2160, OoT3D): bug report "opening and closing bug report seems to
leak something and makes the game lag the more you open and close it"
(`<user>/bug-reports/20261127-124503-opening-and-closing-bug-report-seems-to-leak-som`).

## Cause
Opening the report writes a savestate, and a savestate clears the GPU's caches
(`core.cpp` calls `gpu->ClearAll(true)`). The core fork's `RasterizerCache::ClearAll` emptied its page
table without unregistering the surfaces in it, so every cached surface's host texture stayed
allocated: about 520 to 790 MiB of VRAM per report at 3840x2160, until the driver paged and the game
lagged.

## Fix
Azahar fork 3f4e582b7: `ClearAll` unregisters every surface in the page table, as `UnregisterAll`
does, so they go to the garbage collector. Twelve report rounds at 3840x2160 hold VRAM at 2.0 to
2.5 GiB; resident memory levels off at about 1.6 GiB after 15 rounds.
