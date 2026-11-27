# A line under the F key

User, 2026-11-27 (Fedora, 3840x2160, OoT3D, Kokiri Forest): bug report "texture seam under F key"
(`<user>/bug-reports/20261127-124315-texture-seam-under-f-key`).

A thin dark vertical line runs down from the F key's left edge, the height of the View eye beside
it. It shows with the keyboard last (`device keyboard` on the report's state).

## Cause
`tools/z3dmods/hud_icons.py` `_png` cut a piece's cell from the texture pack's 8x copy of
`menu_top_parts00`. The pack's upscaled art spills a few pixels over the game's cell edges: the grey
bar left of the eye's cell reaches 6 pixels into it (alpha 89 down to 45), while the game's own
sheet is empty there. The cut `view.png` carried that spill as a column at its left edge, drawn
the eye's full height below the key cap. Most item icons carried a neighbour's spill along an edge
the same way.

## Fix
A cell cut from the pack keeps only the ink joined to the game's own art in that cell (`_Sheet.art`):
the pack's ink is grown by two pixels, flood-filled from each of the game's ink pixels, and the
rest is cleared. Removed pixels across both games' 443 pieces and icons are edge spills and
isolated specks; Navi's glow, the sword's tip and the card's outline past the game's box stay.
The report's state with the keyboard shows the eye without the line.
