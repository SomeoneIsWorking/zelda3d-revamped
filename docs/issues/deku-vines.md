# Deku Tree vines in the OoT3D pack

Reported 2026-11-27, Fedora, OoT3D with Henriko Magnifico 5.0a (1080p), after the chest fix:

> Okay, umm can you also check the vines inside deku tree?

## Checked

The game's textures were dumped inside the Deku Tree (entrance 0x0, `--no-textures --dump-textures`).
Each was paired with the pack's image by legacy hash and overlaid
(`z3dmods packcheck`, plus alpha overlays at the game's size). Shots were taken of the same frame
with the pack on and off.

- `ENVIRONMENT/tex1_256x256_8BC39A50612FF5DA_13_mip0.png`, the climbable vine wall. The pack's
  alpha silhouette lies on the game's: the strands, holes and bottom fringe sit on the same texels.
  The pack paints the strands thinner, with dark shading between them inside the same opaque area,
  and yellower, so in the game the wall reads as thin strands over more bark. That is the pack's
  style, not a layout error.
- The same texture's right column (x 180-256) holds small plant sprites, and the pack redrew them as
  different plants. Rows 0-37 of the game's ferns with cyan tips are empty in the pack, so any face
  sampling them shows nothing. The brown stalk and grass blades below became lavender and ivy, the
  leafy vine network became ferns, and the white flowers became grey sprigs. The positions match,
  except that the empty top rows have nothing in them at all.
- `ENVIRONMENT/tex1_256x256_D8AEAA8DA8F551D8_13_mip0.png`, the hanging moss and vines under the
  ledges. The alpha matches the game's to the texel. The pack fills the hanging stems with moss
  instead of drawing leafy stems with hooked ends.
- The slatted gate (`762EDC2ED8E718FB`) and the spider web (`BA78BEA733EEFE64`) are redrawn
  designs: rope bars on the gate and a different radial web.

## State

No layout error like the chest's was found in the vine textures. Still open: which faces sample the
sprite column's empty top rows, so whether any plant is missing in play.
