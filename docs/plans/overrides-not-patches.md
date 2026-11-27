# Overrides instead of memory patches

User, 2026-11-26 (Fedora): "Replace these patches with overrides", "I mean override functions
instead of memory patching", "I don't like memory patches", "I think instead of patching memory to
move items, we instead override the function that builds the menu and make it build the menu in
the way we want instead".

A change to the game's behaviour is a function of ours that replaces the game's at its entry
(`hooks.s`: a branch at the first instruction, checked against the game's word), never a patched
literal, table entry or call site in the middle of a function. Where ours keeps the game's work,
it runs the game's through a trampoline (the first instruction, then a branch to the second) and
changes the result.

## Where it stands
- OoT3D pause screens: done. `ItemMenu_Init`, `GearMenu_Init`, `MapMenu_Init`, `SaveMenu_Init` and
  file select's `FileChoose_InitQuads` are overridden (`pause_screens.cpp`, `top_touch_screen.cpp`):
  the game's builder runs, then `QuadBatch_SetRects` places BACK, the tabs, the bar, the panel and
  the column ours. No table is patched. The grid's cursor and stylus are the overridden
  `ItemMenu_NodePosition`, `ItemMenu_PlaceCursor`, `ItemMenu_UpdateCursor`, `ItemMenu_UpdateTouch`
  and `Touch_InRect` (`item_grid.cpp`, `pause_screens.cpp`).
- OoT3D play: the cutscene interpreter (the Start skip), `Play_Update` (our per-frame features in
  front of the game's) and `Camera_Update` (the free camera after the game's) are overridden in
  place of their call sites.
- OoT3D item screen: `ItemMenu_Draw` is ours, the game's pieces in its order with our inventory
  and slot column between them, and `HudItemButtons_UpdateAmmo` is overridden to move the grid's
  button nodes off the screen when the layout is the item screen's.
- Open: the rest of both games' `hooks.s` sites that are not entry overrides are call hooks and
  literal patches inside functions: cutscene skip, camera, text, The One Screen's `base_*` sites
  and others. Each becomes an override of its function, one function at a time.
