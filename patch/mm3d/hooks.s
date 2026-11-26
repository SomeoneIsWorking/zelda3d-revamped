.syntax unified
.arm
@ Addresses are in Project Restoration's code layout (v1.0), which its v110 patch produces.
@ Cutscene update, first frame of a script.
.section .patch.interpreter_start, "ax", %progbits
    bl _ZN7z3dmods4mm3d12stepCutsceneEPvS1_PKv
.section .expect.interpreter_start, "", %progbits
    .word 0xEB0040CE @ bl 0x1846B4
@ State 1 handler.
.section .patch.interpreter_state1, "ax", %progbits
    bl _ZN7z3dmods4mm3d12stepCutsceneEPvS1_PKv
.section .expect.interpreter_state1, "", %progbits
    .word 0xEBF2D32F @ bl 0x1846B4
@ State 2 and 4 handler: the per-frame tail call into the interpreter.
.section .patch.interpreter_tail, "ax", %progbits
    b _ZN7z3dmods4mm3d12stepCutsceneEPvS1_PKv
.section .expect.interpreter_tail, "", %progbits
    .word 0xEAF64B0E @ b 0x1846B4
@ Actor_UpdateAll's first call, once a frame in play; r7 is the play state.
.section .patch.actor_update, "ax", %progbits
    bl actor_update
.section .expect.actor_update, "", %progbits
    .word 0xEBFFE947 @ bl 0x16EBAC
@ Play_SaveAtStatue's first instruction: both statues' saves pass here.
.section .patch.save_entry, "ax", %progbits
    b save_entry
.section .expect.save_entry, "", %progbits
    .word 0xE92D41F0 @ push {r4-r8, lr}
@ Play_Update's first instruction, every frame of a play state, a touch screen up or not; r0 is
@ the play state.
.section .patch.play_entry, "ax", %progbits
    b play_entry
.section .expect.play_entry, "", %progbits
    .word 0xE92D4FF0 @ push {r4-r11, lr}
@ Camera_Follow's first instruction, the camera the C-stick turns; r0 is the camera.
.section .patch.camera_follow, "ax", %progbits
    b camera_follow
.section .expect.camera_follow, "", %progbits
    .word 0xE92D4FF0 @ push {r4-r11, lr}
@ Input_CStickNeutral's first instruction; r0 is the play state's input.
.section .patch.cstick_neutral, "ax", %progbits
    b cstick_neutral
.section .expect.cstick_neutral, "", %progbits
    .word 0xE2800058 @ add r0, r0, #0x58
@ MessagePrinter_Update's first instruction; r0 is the printer, s0 the frame's seconds.
.section .patch.printer_update, "ax", %progbits
    b printer_update
.section .expect.printer_update, "", %progbits
    .word 0xE92D4010 @ push {r4, lr}
@ MainScreen_Draw's call of MainScreen_DrawMinimap; r0 is the play state (minimap.cpp).
.section .patch.minimap, "ax", %progbits
    bl _ZN7z3dmods4mm3d11drawMinimapEPv
.section .expect.minimap, "", %progbits
    .word 0xEBF41FC7 @ bl 0x2A326C
@ Target_Draw's turn of the arrow: r0 is the angle; arrow_turn leaves it in s0 as a float.
.section .patch.arrow_turn_move, "ax", %progbits
    bl arrow_turn
.section .expect.arrow_turn_move, "", %progbits
    .word 0xEE000A10 @ vmov s0, r0
.section .patch.arrow_turn_convert, "ax", %progbits
    nop
.section .expect.arrow_turn_convert, "", %progbits
    .word 0xEEB80A40 @ vcvt.f32.u32 s0, s0
.text
@ s1 holds the arrow's unit, which the game multiplies by next.
arrow_turn:
    push {r4, lr}
    vpush {d0}
    bl _ZN7z3dmods4mm3d9arrowTurnEj
    vpop {d0}
    vmov s0, r0
    pop {r4, pc}
printer_update:
    push {r0-r3, r12, lr}
    vpush {d0}
    bl _ZN7z3dmods4mm3d16quickTextPrinterEPv
    vpop {d0}
    pop {r0-r3, r12, lr}
    push {r4, lr}
    b kPrinterUpdate + 4
cstick_neutral:
    push {r0-r3, r12, lr}
    bl _ZN7z3dmods4mm3d12mouseTurningEv
    cmp r0, #0
    pop {r0-r3, r12, lr}
    movne r0, #0
    bxne lr
    add r0, r0, #0x58
    b kCStickNeutral + 4
camera_follow:
    push {r0-r3, r12, lr}
    vpush {d0-d7}
    bl _ZN7z3dmods4mm3d17turnCameraByMouseEPv
    vpop {d0-d7}
    pop {r0-r3, r12, lr}
    push {r4, r5, r6, r7, r8, r9, r10, r11, lr}
    b kCameraFollow + 4
play_entry:
    push {r0-r3, r12, lr}
    bl _ZN7z3dmods4mm3d10updatePlayEPv
    pop {r0-r3, r12, lr}
    push {r4, r5, r6, r7, r8, r9, r10, r11, lr}
    b kPlayUpdate + 4
save_entry:
    push {r0-r3, r12, lr}
    bl _ZN7z3dmods4mm3d10beforeSaveEv
    pop {r0-r3, r12, lr}
    push {r4, r5, r6, r7, r8, lr}
    b kPlaySaveAtStatue + 4
actor_update:
    push {r3, lr}
    bl _ZN7z3dmods4mm3d11hideBaseHudEv
    pop {r3, lr}
    b kActorUpdateFirstCall
@ Graph_DrawTopScreen's draw of queue 1 (top_touch_screen.cpp).
.section .patch.top_queue_1, "ax", %progbits
    bl _ZN7z3dmods4mm3d13drawTopQueue1EPviS1_
.section .expect.top_queue_1, "", %progbits
    .word 0xEB0056C6 @ bl 0x126810
@ MessageMgr_DrawWindow's queueing of play's message box, which the item screens place
@ (item_screens.cpp).
.section .patch.message_window, "ax", %progbits
    bl _ZN7z3dmods4mm3d17drawMessageWindowEPvS1_ii
.section .expect.message_window, "", %progbits
    .word 0xEB000006 @ bl 0x17EC58
@ The Items and Masks screens' queueing of their touch layouts: each screen's page and cells, and
@ ItemScreen_DrawCursor's cursors (item_screens.cpp).
.section .patch.items_page, "ax", %progbits
    bl _ZN7z3dmods4mm3d13drawItemsPageEPvS1_ii
.section .expect.items_page, "", %progbits
    .word 0xEBEF8213 @ bl 0x17EC58
.section .patch.items_cell, "ax", %progbits
    bl _ZN7z3dmods4mm3d12drawItemCellEPvS1_ii
.section .expect.items_cell, "", %progbits
    .word 0xEBEF81E5 @ bl 0x17EC58
.section .patch.masks_page, "ax", %progbits
    bl _ZN7z3dmods4mm3d13drawMasksPageEPvS1_ii
.section .expect.masks_page, "", %progbits
    .word 0xEBEF7D95 @ bl 0x17EC58
.section .patch.masks_cell, "ax", %progbits
    bl _ZN7z3dmods4mm3d12drawItemCellEPvS1_ii
.section .expect.masks_cell, "", %progbits
    .word 0xEBEF7D65 @ bl 0x17EC58
.section .patch.item_cursor, "ax", %progbits
    bl _ZN7z3dmods4mm3d14drawItemCursorEPvS1_ii
.section .expect.item_cursor, "", %progbits
    .word 0xEBEF748C @ bl 0x17EC58
@ GearScreen_Draw's queueing of its page and its cursor (gear_screen.cpp).
.section .patch.gear_page, "ax", %progbits
    bl _ZN7z3dmods4mm3d12drawGearPageEPvS1_ii
.section .expect.gear_page, "", %progbits
    .word 0xEBEF6330 @ bl 0x17EC58
.section .patch.gear_cursor, "ax", %progbits
    bl _ZN7z3dmods4mm3d14drawGearCursorEPvS1_ii
.section .expect.gear_cursor, "", %progbits
    .word 0xEBEF6328 @ bl 0x17EC58
@ TitleScreen_Draw's tail call queueing PRESS START.
.section .patch.title_press_start, "ax", %progbits
    b _ZN7z3dmods4mm3d19drawTitlePressStartEPvS1_ii
.section .expect.title_press_start, "", %progbits
    .word 0xEAEF5A8D @ b 0x17EC58
@ FileSelectScreen_Draw's queueing of the file's details (the Start page's plate), of its title bar,
@ which the list replaces, of the list, and of its prompts (a tail call).
.section .patch.file_details, "ax", %progbits
    bl _ZN7z3dmods4mm3d15drawFileDetailsEPvS1_ii
.section .expect.file_details, "", %progbits
    .word 0xEBEF12C9 @ bl 0x17EC58
.section .patch.file_title_bar, "ax", %progbits
    nop
.section .expect.file_title_bar, "", %progbits
    .word 0xEBEF12C1 @ bl 0x17EC58
.section .patch.file_list, "ax", %progbits
    bl _ZN7z3dmods4mm3d12drawFileListEPvS1_ii
.section .expect.file_list, "", %progbits
    .word 0xEBEF129F @ bl 0x17EC58
.section .patch.file_dialogs, "ax", %progbits
    b _ZN7z3dmods4mm3d15drawFileDialogsEPvS1_ii
.section .expect.file_dialogs, "", %progbits
    .word 0xEAEF1296 @ b 0x17EC58
@ Input_ReadTouch's read of the stylus (top_touch_screen.cpp).
.section .patch.read_touch, "ax", %progbits
    bl _ZN7z3dmods4mm3d9readTouchEPvS1_
.section .expect.read_touch, "", %progbits
    .word 0xEB00312A @ bl 0x122678
@ FileSelectScreen_Update's call of FileSelectScreen_UpdateStart, and FileSelectScreen's vtable
@ entry for its build (file_start_page.cpp).
.section .patch.file_start_update, "ax", %progbits
    bl _ZN7z3dmods4mm3d15updateFileStartEPvS1_
.section .expect.file_start_update, "", %progbits
    .word 0xEBFFF213 @ bl 0x5B821C
.section .patch.file_select_init, "a", %progbits
    .word _ZN7z3dmods4mm3d14initFileSelectEPvS1_
.section .expect.file_select_init, "", %progbits
    .word 0x005BA3BC @ FileSelectScreen_Init
@ ItemScreen_DrawBase's queueing of unk_68, unk_70, the column's icons and the held ones
@ (item_screens.cpp).
.section .patch.item_base_68, "ax", %progbits
    bl _ZN7z3dmods4mm3d12drawItemBaseEPvS1_ii
.section .expect.item_base_68, "", %progbits
    .word 0xEBEEF2AB @ bl 0x17EC58
.section .patch.item_base_70, "ax", %progbits
    bl _ZN7z3dmods4mm3d12drawItemBaseEPvS1_ii
.section .expect.item_base_70, "", %progbits
    .word 0xEBEEF2A3 @ bl 0x17EC58
.section .patch.item_base_icon, "ax", %progbits
    bl _ZN7z3dmods4mm3d18skipItemButtonIconEPvS1_ii
.section .expect.item_base_icon, "", %progbits
    .word 0xEBEEF275 @ bl 0x17EC58
.section .patch.item_base_held, "ax", %progbits
    bl _ZN7z3dmods4mm3d12drawItemBaseEPvS1_ii
.section .expect.item_base_held, "", %progbits
    .word 0xEBEEF266 @ bl 0x17EC58
@ OptionsScreen's vtable entry for its update, which hands over to the app's options
@ (options_handoff.cpp).
.section .patch.options_update, "a", %progbits
    .word _ZN7z3dmods4mm3d13updateOptionsEPvS1_
.section .expect.options_update, "", %progbits
    .word 0x005A3D60 @ OptionsScreen_Update
