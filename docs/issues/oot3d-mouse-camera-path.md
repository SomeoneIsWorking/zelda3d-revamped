# OoT3D: the mouse does not turn the camera on Kokiri Forest's log walkway

Observed (user, 2026-11-24): on the raised log walkway in the south-west of Kokiri Forest (sword
out, Navi on F) the mouse camera "doesn't always work". Expected: the mouse turns the free camera
everywhere the game's own follow camera runs.

Not reproduced yet. Ruled out:
- The camera function. Link teleported over Kokiri Forest on a 100-unit grid and read once he
  stood still: every grounded point runs `Camera_Normal1` (setting 1, mode 0), the walkway at
  y 210 around z -1300..-1600 included; only a point outside the map ran function 43.
  `FreeCamera::eligible` takes Normal1.
- The pad mode. `gHud.state` reads 2 idle, walking, swinging, with the sword left out and with Z
  held, so the slot link stays in play and the app gives the mouse to the camera.
- The harness: on the walkway the C-stick and the mouse both turn `camDir` (yaw 25636 to -9463
  to -14924).

Next: the windowed app on the user's own save at that spot (a harness-made file can be saved
there by autosave on a scene change), the mouse moved, with `mouse camera:` log lines and
`camDir` read each tick, to see which of the app's job, the patch's `takeLook` and the orbit
drops the turn.
