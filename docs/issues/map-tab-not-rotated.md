# The item screen's Map tab is not rotated

User, 2026-11-27 (Fedora, 3840x2160, OoT3D): bug report "map button not rotated"
(`<user>/bug-reports/20261127-124352-map-button-not-rotated`).

The tabs down the left edge read sideways (Gear, Items), but Map's label stays level.

## Cause
`patch/oot3d/pause_screens.cpp` `initItemMenu` trimmed the Map tab's cell after the tab quads were
built and turned. Setting the cell rewrites its quad's UVs as the game laid them, which undid the turn.

## Fix
The Map cell is trimmed before `itemQuads.build`, so the turn applies to the trimmed UVs. The
item screen in the `pause-wide` input test shows Map sideways like Gear and Items.
