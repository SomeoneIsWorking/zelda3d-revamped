OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_ZN7z3dmods5oot3d12stepCutsceneEPvS1_PKv)
INCLUDE symbols.ld
SECTIONS {
  /* Fixed sites in address order. */
  .patch.static_glyph_cell kTextRendererAddGlyph + 0x150 : { KEEP(*(.patch.static_glyph_cell)) }
  .patch.cutscene_interpreter kCutsceneInterpreter : { KEEP(*(.patch.cutscene_interpreter)) }
  .patch.camera_update kCameraUpdate : { KEEP(*(.patch.camera_update)) }
  .patch.glyph_texture_mode kTextRendererInit + 0x13C : { KEEP(*(.patch.glyph_texture_mode)) }
  .patch.text_element kMessageLayoutText + 0xA8 : { KEEP(*(.patch.text_element)) }
  .patch.instant_off kMessageLayoutText + 0x1B8 : { KEEP(*(.patch.instant_off)) }
  .patch.play_update kPlayUpdate : { KEEP(*(.patch.play_update)) }
  .patch.item_node_position kItemMenuNodePosition : { KEEP(*(.patch.item_node_position)) }
  .patch.item_update_touch kItemMenuUpdateTouch : { KEEP(*(.patch.item_update_touch)) }
  .patch.item_update_cursor kItemMenuUpdateCursor : { KEEP(*(.patch.item_update_cursor)) }
  .patch.item_buttons_update kHudItemButtonsUpdateAmmo : { KEEP(*(.patch.item_buttons_update)) }
  .patch.save_entry kSramWriteSave : { KEEP(*(.patch.save_entry)) }
  .patch.touch_in_rect kTouchInRect : { KEEP(*(.patch.touch_in_rect)) }
  .patch.song_notes kPadRequestData + 0x98 : { KEEP(*(.patch.song_notes)) }
  .patch.touch_projection3 kTouchScreen2DLayer3 + 0x1B8 : { KEEP(*(.patch.touch_projection3)) }
  .patch.touch_projection2 kTouchScreen2DLayer2 + 0x1F0 : { KEEP(*(.patch.touch_projection2)) }
  .patch.touch_projection1 kTouchScreen2DLayer1 + 0x1C4 : { KEEP(*(.patch.touch_projection1)) }
  .patch.touch_projection0 kTouchScreen2DLayer0 + 0x1CC : { KEEP(*(.patch.touch_projection0)) }
  .patch.base_b_ring kTopHudUpdateButtons + 0x40 : { KEEP(*(.patch.base_b_ring)) }
  .patch.base_b_item kTopHudDraw + 0x458 : { KEEP(*(.patch.base_b_item)) }
  .patch.item_menu_draw kItemMenuDraw : { KEEP(*(.patch.item_menu_draw)) }
  .patch.item_place_cursor kItemMenuPlaceCursor : { KEEP(*(.patch.item_place_cursor)) }
  .patch.base_camera kGameplayUpdate + 0xA8 : { KEEP(*(.patch.base_camera)) }
  .patch.gear_menu_init kGearMenuInit : { KEEP(*(.patch.gear_menu_init)) }
  .patch.save_menu_init kSaveMenuInit : { KEEP(*(.patch.save_menu_init)) }
  .patch.static_cell_glyph kTextRendererFillStatic + 0x80 : { KEEP(*(.patch.static_cell_glyph)) }
  .patch.map_menu_init kMapMenuInit : { KEEP(*(.patch.map_menu_init)) }
  .patch.file_choose_init_quads kFileChooseInitQuads : { KEEP(*(.patch.file_choose_init_quads)) }
  .patch.item_menu_init kItemMenuInit : { KEEP(*(.patch.item_menu_init)) }
  .patch.arrow_turn_move kTargetDraw + kArrowTurnMove : { KEEP(*(.patch.arrow_turn_move)) }
  .patch.arrow_turn_convert kTargetDraw + kArrowTurnConvert : { KEEP(*(.patch.arrow_turn_convert)) }
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
