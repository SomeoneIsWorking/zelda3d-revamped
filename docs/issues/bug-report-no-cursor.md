# The bug report says to drag on the picture, but the mouse is not visible

User, 2026-11-27 (Fedora, 3840x2160, OoT3D): bug report "It says drag on a picture bug the mouse
isn't visible" (`<user>/bug-reports/20261127-124528-it-says-drag-on-a-picture-bug-the-mouse-isn-t-vi`).

## Cause
In play `MouseAim` captures the mouse (relative mode, hidden cursor), and the report screen did not
release it, so the cursor stayed hidden over the form.

## Fix
`GameWindow::pollMenu` releases the mouse whenever a menu screen polls; `MouseAim` captures it again
in play. The log reads `mouse released` when the report opens and `mouse captured` after it closes.
