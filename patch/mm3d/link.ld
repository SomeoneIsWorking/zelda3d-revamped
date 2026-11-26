OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_ZN7z3dmods4mm3d12stepCutsceneEPvS1_PKv)
INCLUDE symbols.ld
SECTIONS {
  /* Fixed sites in address order. */
  .patch.top_queue_1 kGraphDrawTopScreen + kGraphDrawTopQueue1Call : { KEEP(*(.patch.top_queue_1)) }
  .patch.read_touch kInputReadTouch + kInputReadTouchCall : { KEEP(*(.patch.read_touch)) }
  .patch.play_entry kPlayUpdate : { KEEP(*(.patch.play_entry)) }
  .patch.interpreter_start kCutsceneBeginScript + 0x1D8 : { KEEP(*(.patch.interpreter_start)) }
  .patch.actor_update kActorUpdateAll + 0x48 : { KEEP(*(.patch.actor_update)) }
  .patch.message_window kMessageMgrDrawWindow + kMessageMgrDrawWindowCall : { KEEP(*(.patch.message_window)) }
  .patch.arrow_turn_move kTargetDraw + kArrowTurnMove : { KEEP(*(.patch.arrow_turn_move)) }
  .patch.arrow_turn_convert kTargetDraw + kArrowTurnConvert : { KEEP(*(.patch.arrow_turn_convert)) }
  .patch.printer_update kPrinterUpdate : { KEEP(*(.patch.printer_update)) }
  .patch.cstick_neutral kCStickNeutral : { KEEP(*(.patch.cstick_neutral)) }
  .patch.save_entry kPlaySaveAtStatue : { KEEP(*(.patch.save_entry)) }
  .patch.interpreter_tail kCutsceneRunScript + 0x28 : { KEEP(*(.patch.interpreter_tail)) }
  .patch.camera_follow kCameraFollow : { KEEP(*(.patch.camera_follow)) }
  .patch.interpreter_state1 kCutsceneStartScript + 0x2C : { KEEP(*(.patch.interpreter_state1)) }
  .patch.map_page kMapScreenDraw + kMapScreenDrawPageCall : { KEEP(*(.patch.map_page)) }
  .patch.map_info kMapScreenDraw + kMapScreenDrawInfoCall : { KEEP(*(.patch.map_info)) }
  .patch.minimap kMainScreenDraw + 0xCC : { KEEP(*(.patch.minimap)) }
  .patch.items_page kItemScreenDraw + kItemScreenDrawPageCall : { KEEP(*(.patch.items_page)) }
  .patch.items_cell kItemScreenDraw + kItemScreenDrawCellCall : { KEEP(*(.patch.items_cell)) }
  .patch.masks_page kMaskScreenDraw + kMaskScreenDrawPageCall : { KEEP(*(.patch.masks_page)) }
  .patch.masks_cell kMaskScreenDraw + kMaskScreenDrawCellCall : { KEEP(*(.patch.masks_cell)) }
  .patch.item_cursor kItemScreenDrawCursor + kItemScreenDrawCursorCall : { KEEP(*(.patch.item_cursor)) }
  .patch.gear_page kGearScreenDraw + kGearScreenDrawPageCall : { KEEP(*(.patch.gear_page)) }
  .patch.gear_cursor kGearScreenDraw + kGearScreenDrawCursorCall : { KEEP(*(.patch.gear_cursor)) }
  .patch.ocarina_play kOcarinaScreenDraw + kOcarinaScreenDrawPlayCall : { KEEP(*(.patch.ocarina_play)) }
  .patch.ocarina_list kOcarinaScreenDraw + kOcarinaScreenDrawListCall : { KEEP(*(.patch.ocarina_list)) }
  .patch.ocarina_cursor kOcarinaScreenDraw + kOcarinaScreenDrawCursorCall : { KEEP(*(.patch.ocarina_cursor)) }
  .patch.title_press_start kTitleScreenDraw + kTitlePressStartCall : { KEEP(*(.patch.title_press_start)) }
  .patch.file_details kFileSelectScreenDraw + kFileSelectDetailsCall : { KEEP(*(.patch.file_details)) }
  .patch.file_title_bar kFileSelectScreenDraw + kFileSelectTitleBarCall : { KEEP(*(.patch.file_title_bar)) }
  .patch.file_list kFileSelectScreenDraw + kFileSelectFilesCall : { KEEP(*(.patch.file_list)) }
  .patch.file_dialogs kFileSelectScreenDraw + kFileSelectDialogsCall : { KEEP(*(.patch.file_dialogs)) }
  .patch.file_start_update kFileSelectScreenUpdate + kFileSelectUpdateStartCall : { KEEP(*(.patch.file_start_update)) }
  .patch.item_base_68 kItemScreenDrawBase + kItemScreenDrawBase68Call : { KEEP(*(.patch.item_base_68)) }
  .patch.item_base_70 kItemScreenDrawBase + kItemScreenDrawBase70Call : { KEEP(*(.patch.item_base_70)) }
  .patch.item_base_icon kItemScreenDrawBase + kItemScreenDrawBaseIconCall : { KEEP(*(.patch.item_base_icon)) }
  .patch.item_base_held kItemScreenDrawBase + kItemScreenDrawBaseHeldCall : { KEEP(*(.patch.item_base_held)) }
  .patch.options_update kOptionsScreenVtable + kOptionsScreenUpdateSlot * 4 : { KEEP(*(.patch.options_update)) }
  .patch.file_select_init kFileSelectScreenVtable + kFileSelectScreenInitSlot * 4 : { KEEP(*(.patch.file_select_init)) }
  . = PAYLOAD;
  .payload : { KEEP(*(.options)) *(.text*) *(.rodata*) *(.data*) }
  /* The item slots' block the app reads and writes (tools/z3dmods/slot_link.py). */
  .link : { KEEP(*(.link)) }
  .payload_zero (NOLOAD) : { *(.bss*) *(COMMON) }
  /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment) *(.note*) *(.eh_frame*) }
}
