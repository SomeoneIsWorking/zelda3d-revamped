# Our own single screen (MM3D; OoT3D as fallback)

User: "single screen exists"; on the CC BY-NC-ND and fork routes: "I'd rather make MIT versions of
those then".

## MM3D
User, 2026-11-13: "Can't you copy the RE? Anyway, do what you can please", then on building
DekUI: "Look, no, no Qt".

DekUI (GPL-2.0) is not usable: it is its own Azahar fork whose single screen is host code in the
emulator (`src/mm3d/`, native inventory, file select, notebook and minimap drawn by the frontend),
and it builds only with the Qt or Android frontend. Its source (v1.1.0, `5cf063e`, read only) is evidence for which guest functions and state each menu uses;
its code is not copied.

User, 2026-11-13: "this needs to be an app using Azahar libretro underneath": single screen moves
to the packed app (docs/plans/packed-app.md), drawn by the app. The code-patch route below is the
fallback for plain Azahar.

Status (2026-11-14): the app's `overlay` layout is MM3D's set default (docs/app.md, "Overlay
layout"): pieces of the game's own touch screen drawn on the top screen from a table,
`patch/mm3d/overlay.toml`, by which touch screen is up (`gUiScreenContext.activeScreen`,
mm3d-decomp `docs/ui.md`): the HUD's elements one by one in play, the whole screen in a menu. The
user rejected the first form, the whole touch screen as one inset with its empty middle panel.
It is still the game's touch screen composited; the HUD drawn by us is plans/pc-controls-hud.md.

The user rejected the play elements cut out of the touch screen as well (2026-11-14: "do this
properly RE based instead of copying patches of bottom screen"). Play gets our own HUD drawn from
the game's state (plans/pc-controls-hud.md, "Rule"); the cut-out pieces are deleted when it lands.
The menus showing the touch screen whole is a stopgap.

Done for play (2026-11-14): `patch/mm3d/overlay.toml`'s `play` layout has no pieces; the HUD is
`patch/mm3d/hud.toml` (`mm-hud2`). What it does not draw yet is listed in plans/pc-controls-hud.md.

Seen (`mm-ov3`): play has the elements along the edges, Items and Masks are large, the area's
title card draws the top screen alone (`hudVisibility` 2). The pad reaches the touch-only buttons
through Project Restoration: ZL is View, ZR with X is item I and ZR with Y is item II (`mm-zl`,
`mm-zr3`).

Items and Masks (2026-11-26) are no longer the touch screen pasted large: the game draws them on
the top screen (`patch/mm3d/item_screens.cpp`), the grid against the app's slots, the tabs down the
left, Return in the corner and the item's description under the grid (`items21`, `inputtest
masks-mm3d`, `slots-mm3d`). Gear (2026-11-26) is drawn there too, its page in the middle under
the title bar with Return in the corner and Options under it (`patch/mm3d/gear_screen.cpp`,
`gear4` to `gear6`). The Map screen's touch page lies over the top screen's map, the loupe under
the pointer and A's button in the corner (`patch/mm3d/map_screen.cpp`, `map5` to `map8`).

The ocarina's pages (2026-11-26) are drawn by the game in the top screen's top left corner at
0.55, clear of Link, with Cancel in the bottom left corner (`patch/mm3d/ocarina_screen.cpp`,
`oc4` to `oc6`, `inputtest slots-mm3d`).

The Bombers' Notebook (table entry 2) stays side by side: its list fills the top screen (400 by
210 under the title bar) and the entry with Return and Schedule fills the touch screen (320 by
213), so one screen would halve both (`mm-pad2`, `note2`).

Left:
- The Song of Soaring map and the Pictograph Box have not been seen in this layout.
- The map tabs have no pad button of their own; Project Restoration opens Items with Select, the
  notebook with Start, Gear with ZR and Start, the map with ZR and Select and the ocarina with ZR
  and A (`mm-pad1`).
- The minimap is not drawn in play: the panel it shares is empty without the area's map, and what
  says the map is owned is not found yet. The black box under Gear is not drawn either; what it
  shows is not known.
- The hearts keep the touch screen's dark strip behind them until the HUD is ours.

Fallback: an MIT code patch that moves the bottom-screen UI (items, masks, map, notebook, Ocarina,
file select) onto the top screen in the game's own code, in the same `code.ips` as the skip, on
Project Restoration's v1.0-layout `code.bin`, built from mm3d-decomp and PR's source (GPL, evidence
only). It does not fork Azahar.

## OoT3D (fallback only)
If The One Screen cannot be used, the same approach for OoT3D, built on zeldaret/oot3d (CC0), with
the OoT3D Randomizer's loader as evidence only (no licence for its own code), in the same `code.ips` as the skip and Quick Text.
