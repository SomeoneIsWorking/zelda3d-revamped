# OoT3D: file select's fade stops at the touch screen's edges

Observed (user, 2026-11-24): after Start on a file, the dim covers the middle of the screen only;
the sides and BACK stay bright. Expected: the whole screen fades.

Reproduce: `z3dmods run oot3d fade2 --harness --resolution 2 wait:20 start wait:3 a wait:3 a
wait:0.2 shot:f1`. The dim covers framebuffer columns 80 to 720 of 800: touch screen x 0 to 320
under z3d-mods' widened projection (x -40 to 360, `patch/oot3d/file_select.cpp`), which is also
The One Screen's own viewport (rows 40 to 360) before z3d-mods widened it.

Ruled out:
- File select's quad batch (`quadSizes`, 0x5055B8): no quad wider than 256.
- The full touch-screen quad (0,0)-(320,240): one copy in the heap (0x0876A350 in that run) and
  two in the linear heap; all three widened to x -40..360 mid-fade, the dim unchanged.

Not settled:
- The One Screen binds the touch-screen pass with rows 40 to 360 in two more places, 0x616498 and
  0x616CD8 (`source/render/presentation.c`'s generic 0x401 path and one other). Writing them
  mid-fade changed nothing, but the CPU's translated code was not flushed, so that test proves
  nothing either way.

Next: decompile file select's confirm state (what Start starts) to the routine that draws the
fill, and widen that at its owner.
