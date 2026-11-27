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

User, 2026-11-27 (Fedora), measuring the two atlases side by side (each at 512 pixels): the front
panel spans 144 in the game's and 146.5 in the pack's, the middle column's arch 33 against 27.5 and
its plank 69 against 58.5. The pack's middle column is drawn about 15% narrower than the game's, so
everything right of the front panel sits off the game's layout, not only the right quarter.

User, 2026-11-27 (Fedora): "this, 27.5 vs 33 looks to be the worst offender here, this pack is
probably not meant for this version of the game". Against it: the core takes a pack file whose name
is the hash of the game texture's bytes (this pack: the legacy hash, `use_new_hash` false), and
replacing `648826B7F68E056C` with a probe changed the chest, so the game's texture is byte for byte
the one the pack was made from; the pack's folder is this title id (USA). The art was redrawn off
the game's layout for this same texture.

User, 2026-11-27 (Fedora), measuring the middle column whole at 78.5 in the game's against 67.5 in
the pack's: "I don't know how this could happen or how someone would make this mistake, it doesn't
feel right, maybe we try another pack".

## Cause
The pack's chest atlas (`ENVIRONMENT/tex1_128x128_648826B7F68E056C_13_mip0.png`) is drawn off the
game's layout of the texture from column 72 rightward: the middle column (arches and plank) is 23
texels wide against the game's 28, the dark gap after it 6 against 8, and the strip right of it
starts 6 texels early, 20 wide against 14, with its dark band and stone 5 texels high (stone from
row 67 against 72). The game samples the image at its own coordinates, so the lid's band took the
column's edge and the corner by the lid took the strip's stone. A per-texel probe (red and green
the texel's x and y, divided by a white run for the lighting) showed the corner reading texels
x 116 to 125, rows 67 to 71: dark in the game's, stone in the pack's.

## Fix
`tools/z3dmods/pack_fixes.py` rebuilds the image from the pack's file on install with its columns
and the strip's rows moved onto the game's (knots fitted against the game's texture,
`sets/pack-fixes/henriko-oot3d-5.0a.toml`), into `z3dmods-pack-fixes/` with the pack's copy left
out. The report's state now shows the lid's band with its silver plates and the dark corner, as
the game draws it, at the pack's resolution; the far chest is unchanged. The pack's own art in the
strip below row 72 (a framed panel where the game has a latch on stone) is kept as drawn.
