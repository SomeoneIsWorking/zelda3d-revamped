# The tools' runs play sound

User, 2026-11-27 (Fedora): "also fyi I can hear your runs"

Input tests (gamescope) and the nested Wayland check run the app windowed, so the core plays
through cubeb on the user's speakers. A tool's run is silent.

User, 2026-11-27 (Fedora): "Silent AND unbounded speed please where applicable, test runs don't
need to wait for vsync imo"

## Fix
`Z3D_SILENT=1` (or `--hidden`) gives the core the null sink, and `Z3D_UNPACED=1` (or `--hidden`)
turns off the frame limiter, vsync and asynchronous texture loading. Input tests always set
`Z3D_SILENT`, and set `Z3D_UNPACED` for the scenarios in `input_test.UNPACED`, which wait on screens
rather than on time. The nested Wayland check sets `Z3D_SILENT`. The report scenario runs at about
270 frames a second.
