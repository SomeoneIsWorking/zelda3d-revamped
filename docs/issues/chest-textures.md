# Chest textures a bit messed up

User, 2026-11-27 (Fedora, 3840x2160, OoT3D, Link's house): bug report "chest textures a bit
messed up" (`<user>/bug-reports/20261127-124331-chest-textures-a-bit-messed-up`).

User, 2026-11-27 (Fedora): "maybe it's not the lid, what's that near the chest body? Can you pull
the textures?" with a crop of the chest's front right corner, where the lid meets the body.

## Investigation
The chest draws from one 128x128 atlas (dumped as `tex1_128x128_9905ACF5E7261618_13`, the pack's
`ENVIRONMENT/tex1_128x128_648826B7F68E056C_13_mip0.png`, 512x512). The report's frame at render
scale 9 with the pack and without: the corner face is the game's dark metal latch without the pack
and a flat grey patch with it. The pack follows the game's layout of the atlas except its right
quarter: where the game has a dark metal latch on stone (u 0.92 to 0.97, v 0.69 to 0.86) the pack
has stone and a framed wood panel, and its wood panel beside the band's plates (v 0 to 0.55) is
framed wider than the game's. Faces mapped there take the pack's stone and panel: the grey patch on
the corner and the panel that looks like a block on the lid. The geometry is the same both ways (a
probe texture of coloured cells shows a plain box). The cause is the pack's art in that quarter.
