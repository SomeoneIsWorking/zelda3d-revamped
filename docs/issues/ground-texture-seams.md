# Visible seams in the ground's texture

User, 2026-11-27 (Fedora, 3840x2160, OoT3D, Kokiri Forest): bug report "visible texture seams"
(`<user>/bug-reports/20261127-124217-visible-texture-seams`).

The grass shows straight diagonal lines where its triangles meet: the texture does not continue
across them.

The user, the same day, on the Deku Tree vines (`deku-vines.md`): "I remember seeing misaligned
segments, like the grass issue I reported earlier". Then, while this was being traced: "probably an
emulator bug", and "Why don't you use a debugger?"

## Investigation

Reproduced on the report's state, with the pack and without it: the lines are there in both,
fainter in the game's own art. A probe that swapped the texture for a solid colour showed the
ground is `s04_yuka_01r` (256x256 ETC1, pack file `ENVIRONMENT/tex1_256x256_9FAD043B4862D0A8_12`).

- **The model.** `scene/spot04_0_info.zsi`, mesh 3: every triangle's texture coordinates sit inside
  one 128-texel cell of the texture (a quarter), 1 texel in from the cell's edges (0.004..0.496,
  0.504..0.996), wrap `GL_REPEAT`. Across a shared edge the two triangles are a whole number of cells
  apart, give or take those 2 inset texels (offsets of 2, 126, 128, 254 texels).
- **The probe.** A texture coding the position inside each cell and the cell's index (with a grey one
  to divide out the lighting) shows a seam's two sides reading opposite edges of a cell (one side at
  about texel 125.7, the other at about 1.3, in cells (0,0) and (1,1)). Those are the model's own
  coordinates. A real 3DS reads the same ones, so the line is not an emulator defect.
- **The game's art.** Every cell edge of the game's texture carries one strip: column 127 against 0,
  128 or 255 differs by about 3 (unrelated columns about 15), and the art is smoothed for a few
  texels toward each edge. That hides the 2-texel jump.
- **The pack.** The pack's cell edges differ from each other by 7 to 8 (at 1024 px, against 4.7 for
  neighbouring columns), so the pack does not keep that rule.

## Tried, not working

Moving every coordinate 1 texel in from a cell edge onto the edge (room files, at install), and
rebuilding the pack image so that every cell edge fades into one seamless tile, did not remove the
lines. Two boots compared with the same steps, HEAD against the change, both showed them. Reverted.
The harness runs before those boots were not valid tests: the state kept the old room in RAM, and a
state from before the install read stale RomFS offsets (`docs/app.md`, harness user data).

Next: probe the changed ground from a daytime boot, to measure what each side of a remaining line
reads after the snap. Either the snap missed those corners, or the cause is something other than
the inset.
