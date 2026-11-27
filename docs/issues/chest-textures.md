# Chest textures a bit messed up

User, 2026-11-27 (Fedora, 3840x2160, OoT3D, Link's house): bug report "chest textures a bit
messed up" (`<user>/bug-reports/20261127-124331-chest-textures-a-bit-messed-up`).

## Investigation
The report's state, shot at render scale 9 with the pack and without: without it the lid's front
is the game's metal band with its two silver plates; with it the same faces show a wood panel
framed in dark bands that sits like a block stuck on the lid. The geometry and the game's UVs are
the same in both, so the pack's replacement texture for the lid's front does not follow the game's
layout of that texture. The cause is in the pack's art, not our code.
