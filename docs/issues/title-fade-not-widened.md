# The title screen's fade is not widened

User, 2026-11-27 (Fedora, 3840x2160, OoT3D): bug report "fade not widened"
(`<user>/bug-reports/20261127-124044-fade-not-widened`).

On the title (PRESS START) the fade darkens the 3DS's width in the middle; the widened strips at
both sides stay lighter.

## Investigation
The report's state (header restamped for core 3f4e582b7) replays it: game mode 1 (title), so
`widened()` is false. Three quads are queued in The One Screen's queue (0x5C0BA8, items at +0xF4)
each frame, all vtable 0x4EBD60, size at +0xC and +0x20, fill strength at +0xFC: a 400x240 quad
(the top screen's own fade) and two 320x240 touch-screen quads. The first two dim together (+0xFC
0.455 to 0.416 over six frames). None reaches The One Screen's lane-6 promotion (0x616B9C; its list
at 0x64B84C stays empty), so the touch quad is drawn through the touch viewport, rows 40 to 360, on
top of the top screen's own full-width fade: the middle is dimmed twice. Next: find the touch fade's
drawer and whether the touch layer shows anything on the top screen on the title.
