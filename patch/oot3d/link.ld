OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_ZN7z3dmods5oot3d12stepCutsceneEPvS1_PKv)
INCLUDE symbols.ld
SECTIONS {
  /* Fixed sites in address order. */
  .patch.static_glyph_cell kTextRendererAddGlyph + 0x150 : { KEEP(*(.patch.static_glyph_cell)) }
  .patch.glyph_texture_mode kTextRendererInit + 0x13C : { KEEP(*(.patch.glyph_texture_mode)) }
  .patch.text_element kMessageLayoutText + 0xA8 : { KEEP(*(.patch.text_element)) }
  .patch.instant_off kMessageLayoutText + 0x1B8 : { KEEP(*(.patch.instant_off)) }
  .patch.draw_camera kGameplayDraw + 0x808 : { KEEP(*(.patch.draw_camera)) }
  .patch.update_camera kPlayUpdate + 0x1540 : { KEEP(*(.patch.update_camera)) }
  .patch.item_cursor_right kItemMenuUpdateCursor + kItemMenuCursorRightWrap : { KEEP(*(.patch.item_cursor_right)) }
  .patch.item_cursor_left kItemMenuUpdateCursor + kItemMenuCursorLeftWrap : { KEEP(*(.patch.item_cursor_left)) }
  .patch.item_node_position_top kItemMenuNodePosition + kItemMenuNodePositionTop : { KEEP(*(.patch.item_node_position_top)) }
  .patch.item_node_position_pitch kItemMenuNodePosition + kItemMenuNodePositionPitch : { KEEP(*(.patch.item_node_position_pitch)) }
  .patch.item_node_position_left kItemMenuNodePosition + kItemMenuNodePositionLeft : { KEEP(*(.patch.item_node_position_left)) }
  .patch.item_update_cursor_pitch kItemMenuUpdateCursor + kItemMenuUpdateCursorPitch : { KEEP(*(.patch.item_update_cursor_pitch)) }
  .patch.item_update_cursor_left kItemMenuUpdateCursor + kItemMenuUpdateCursorLeft : { KEEP(*(.patch.item_update_cursor_left)) }
  .patch.item_update_cursor_top kItemMenuUpdateCursor + kItemMenuUpdateCursorTop : { KEEP(*(.patch.item_update_cursor_top)) }
  .patch.item_update_touch_left kItemMenuUpdateTouch + kItemMenuUpdateTouchLeft : { KEEP(*(.patch.item_update_touch_left)) }
  .patch.item_update_touch_inverse_pitch kItemMenuUpdateTouch + kItemMenuUpdateTouchInversePitch : { KEEP(*(.patch.item_update_touch_inverse_pitch)) }
  .patch.item_update_touch_top kItemMenuUpdateTouch + kItemMenuUpdateTouchTop : { KEEP(*(.patch.item_update_touch_top)) }
  .patch.item_update_touch_pitch kItemMenuUpdateTouch + kItemMenuUpdateTouchPitch : { KEEP(*(.patch.item_update_touch_pitch)) }
  .patch.item_place_cursor_pitch kItemMenuPlaceCursor + kItemMenuPlaceCursorPitch : { KEEP(*(.patch.item_place_cursor_pitch)) }
  .patch.item_place_cursor_top kItemMenuPlaceCursor + kItemMenuPlaceCursorTop : { KEEP(*(.patch.item_place_cursor_top)) }
  .patch.item_place_cursor_left kItemMenuPlaceCursor + kItemMenuPlaceCursorLeft : { KEEP(*(.patch.item_place_cursor_left)) }
  .patch.item_back_touch0 kItemMenuUpdate + kItemMenuBackTouchCall0 : { KEEP(*(.patch.item_back_touch0)) }
  .patch.item_back_touch1 kItemMenuUpdate + kItemMenuBackTouchCall1 : { KEEP(*(.patch.item_back_touch1)) }
  .patch.item_back_touch2 kItemMenuUpdate + kItemMenuBackTouchCall2 : { KEEP(*(.patch.item_back_touch2)) }
  .patch.item_items_touch0 kItemMenuUpdate + kItemMenuItemsTouchCall0 : { KEEP(*(.patch.item_items_touch0)) }
  .patch.item_items_touch1 kItemMenuUpdate + kItemMenuItemsTouchCall1 : { KEEP(*(.patch.item_items_touch1)) }
  .patch.item_items_touch2 kItemMenuUpdate + kItemMenuItemsTouchCall2 : { KEEP(*(.patch.item_items_touch2)) }
  .patch.item_gear_touch0 kItemMenuUpdate + kItemMenuGearTouchCall0 : { KEEP(*(.patch.item_gear_touch0)) }
  .patch.item_gear_touch1 kItemMenuUpdate + kItemMenuGearTouchCall1 : { KEEP(*(.patch.item_gear_touch1)) }
  .patch.item_gear_touch2 kItemMenuUpdate + kItemMenuGearTouchCall2 : { KEEP(*(.patch.item_gear_touch2)) }
  .patch.item_map_touch0 kItemMenuUpdate + kItemMenuMapTouchCall0 : { KEEP(*(.patch.item_map_touch0)) }
  .patch.item_map_touch1 kItemMenuUpdate + kItemMenuMapTouchCall1 : { KEEP(*(.patch.item_map_touch1)) }
  .patch.item_map_touch2 kItemMenuUpdate + kItemMenuMapTouchCall2 : { KEEP(*(.patch.item_map_touch2)) }
  .patch.save_entry kSramWriteSave : { KEEP(*(.patch.save_entry)) }
  .patch.interpreter_catchup kCutsceneRunScript + 0xF0 : { KEEP(*(.patch.interpreter_catchup)) }
  .patch.interpreter_tail kCutsceneRunScript + 0x120 : { KEEP(*(.patch.interpreter_tail)) }
  .patch.touch_projection3 kTouchScreen2DLayer3 + 0x1B8 : { KEEP(*(.patch.touch_projection3)) }
  .patch.touch_projection2 kTouchScreen2DLayer2 + 0x1F0 : { KEEP(*(.patch.touch_projection2)) }
  .patch.touch_projection1 kTouchScreen2DLayer1 + 0x1C4 : { KEEP(*(.patch.touch_projection1)) }
  .patch.touch_projection0 kTouchScreen2DLayer0 + 0x1CC : { KEEP(*(.patch.touch_projection0)) }
  .patch.song_notes kPadRequestData + 0x98 : { KEEP(*(.patch.song_notes)) }
  .patch.base_b_ring kTopHudUpdateButtons + 0x40 : { KEEP(*(.patch.base_b_ring)) }
  .patch.base_b_item kTopHudDraw + 0x458 : { KEEP(*(.patch.base_b_item)) }
  .patch.item_grid_update kItemMenuUpdate + kItemMenuUpdateItemsCall : { KEEP(*(.patch.item_grid_update)) }
  .patch.item_menu_draw_model kItemMenuDraw + kItemMenuDrawModelCall : { KEEP(*(.patch.item_menu_draw_model)) }
  .patch.item_menu_draw kItemMenuDraw + kItemMenuDrawLayout40Call : { KEEP(*(.patch.item_menu_draw)) }
  .patch.play_update kGameplayUpdate + 0x9C : { KEEP(*(.patch.play_update)) }
  .patch.base_camera kGameplayUpdate + 0xA8 : { KEEP(*(.patch.base_camera)) }
  .patch.static_cell_glyph kTextRendererFillStatic + 0x80 : { KEEP(*(.patch.static_cell_glyph)) }
  .patch.item_menu_init kItemMenuInit + kItemMenuInitLastCall : { KEEP(*(.patch.item_menu_init)) }
  .patch.arrow_turn_move kTargetDraw + kArrowTurnMove : { KEEP(*(.patch.arrow_turn_move)) }
  .patch.arrow_turn_convert kTargetDraw + kArrowTurnConvert : { KEEP(*(.patch.arrow_turn_convert)) }
  /* File select's quad places (hooks.s back_lit, back_button), two floats a quad. */
  .patch.back_lit kFileChoose + kFileChooseQuadPositions + kFileChooseQuadBackLit * 8 : { KEEP(*(.patch.back_lit)) }
  .patch.back_button kFileChoose + kFileChooseQuadPositions + kFileChooseQuadBack * 8 : { KEEP(*(.patch.back_button)) }
  /* The pause screens' (hooks.s <screen>_back_lit, <screen>_back). */
  .patch.item_back_lit kItemMenuQuadPositions + kItemMenuQuadBackLit * 8 : { KEEP(*(.patch.item_back_lit)) }
  .patch.item_panel kItemMenuQuadPositions + kItemMenuQuadPanel * 8 : { KEEP(*(.patch.item_panel)) }
  .patch.item_tab_bar kItemMenuQuadPositions + kItemMenuQuadTabBar * 8 : { KEEP(*(.patch.item_tab_bar)) }
  .patch.item_map_tab_cell kItemMenuQuadUvs + (kItemMenuQuadTabs + 1) * 8 : { KEEP(*(.patch.item_map_tab_cell)) }
  .patch.item_map_tab_cell_size kItemMenuQuadUvSizes + (kItemMenuQuadTabs + 1) * 8 : { KEEP(*(.patch.item_map_tab_cell_size)) }
  .patch.item_column kItemMenuQuadPositions + kItemMenuQuadColumn * 8 : { KEEP(*(.patch.item_column)) }
  .patch.item_back kItemMenuQuadPositions + kItemMenuQuadBack * 8 : { KEEP(*(.patch.item_back)) }
  .patch.gear_back_lit kGearMenuQuadPositions + kGearMenuQuadBackLit * 8 : { KEEP(*(.patch.gear_back_lit)) }
  .patch.gear_back kGearMenuQuadPositions + kGearMenuQuadBack * 8 : { KEEP(*(.patch.gear_back)) }
  .patch.map_back_lit kMapMenuQuadPositions + kMapMenuQuadBackLit * 8 : { KEEP(*(.patch.map_back_lit)) }
  .patch.map_back kMapMenuQuadPositions + kMapMenuQuadBack * 8 : { KEEP(*(.patch.map_back)) }
  .patch.map_other_back_lit kMapMenuOtherQuadPositions + kMapMenuOtherQuadBackLit * 8 : { KEEP(*(.patch.map_other_back_lit)) }
  .patch.map_other_back kMapMenuOtherQuadPositions + kMapMenuOtherQuadBack * 8 : { KEEP(*(.patch.map_other_back)) }
  .patch.save_back_lit kSaveMenuQuadPositions + kSaveMenuQuadBackLit * 8 : { KEEP(*(.patch.save_back_lit)) }
  .patch.save_back kSaveMenuQuadPositions + kSaveMenuQuadBack * 8 : { KEEP(*(.patch.save_back)) }
  /* The One Screen 891b60e's payload (decomp docs/hud.md). Ocarina_Draw's songs transform: the
     top screen's width it centres the song list in. */
  .patch.base_song_list_width 0x610EB0 : { KEEP(*(.patch.base_song_list_width)) }
  /* Hud_Submit's visible flag. */
  .patch.base_hud 0x612C84 : { KEEP(*(.patch.base_hud)) }
  /* Hud_Draw's call of Ocarina_Draw, the song list on the top screen. */
  .patch.base_ocarina_draw 0x61462C : { KEEP(*(.patch.base_ocarina_draw)) }
  /* Minimap_Project's mounted test. */
  .patch.base_minimap 0x6148C8 : { KEEP(*(.patch.base_minimap)) }
  /* Presentation_Target's file-select pass on the top screen: its viewport's first row and rows. */
  .patch.base_file_select_row 0x616400 : { KEEP(*(.patch.base_file_select_row)) }
  .patch.base_file_select_rows 0x616408 : { KEEP(*(.patch.base_file_select_rows)) }
  .patch.base_fade_draw 0x616cc4 : { KEEP(*(.patch.base_fade_draw)) }
  /* Presentation_Target's viewport for a touch screen it draws on the top screen. */
  .patch.base_touch_viewport 0x6164cc : { KEEP(*(.patch.base_touch_viewport)) }
  /* tos_common_backdrop's draw of the touch screen's backdrop. */
  .patch.base_backdrop_draw 0x6156ec : { KEEP(*(.patch.base_backdrop_draw)) }
  . = PAYLOAD;
  .payload : { KEEP(*(.options)) *(.text*) *(.rodata*) *(.data*) }
  /* The item slots' block the app reads and writes (tools/z3dmods/slot_link.py). */
  .link : { KEEP(*(.link)) }
  .payload_zero (NOLOAD) : { *(.bss*) *(COMMON) }
  /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment) *(.note*) *(.eh_frame*) }
}
