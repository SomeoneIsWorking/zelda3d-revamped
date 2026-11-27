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
- OoT3D file select: `Camera2D_SetMatrices` is overridden and widens the touch screen's camera
  (`gTouchCamera2D`), not four calls in the layers; `TouchScreen2D_Begin`'s pass on that camera is
  widened too, so file select's sky keeps its aspect.
- OoT3D text: `TextRenderer_Init` is overridden to pass the static glyph texture's mode.
- Open, OoT3D game code: each site is inside a function that has to be written whole in C++ to
  become an override. `TextRenderer_AddGlyph` (560 bytes, `static_glyph_cell`),
  `TextRenderer_FillStaticGlyphTexture` (780, `static_cell_glyph`), `PadMgr_RequestPadData` (960,
  `song_notes`), `TopHud_UpdateButtons` (1016, `base_b_ring`), `TopHud_Draw` (1328, `base_b_item`),
  `Message_LayoutText` (2904, Quick Text's `text_element` and `instant_off`) and `Target_Draw`
  (3052, `arrow_turn_*`).
- Open, The One Screen: the ten `base_*` sites patch The One Screen's own code (MIT). Their owner
  is a fork of The One Screen built from source with our changes, installed in place of its
  released `code.ips`.
- Open, MM3D: about 30 call hooks in the screens' draw and update functions (items, masks, gear,
  map, ocarina, title, file select, the minimap, the message window, actors, cutscenes,
  `Input_ReadTouch`, `Target_Draw`); each screen's draw becomes an override of the whole function,
  as OoT3D's `ItemMenu_Draw` did.
