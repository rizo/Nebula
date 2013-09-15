;;; keyboard.asm
;;;
;;; Basic keyboard demo which writes characters to the screen.

;;;
;;; Constants

        .#def empty_style 0
        .#def font_style 0xf000   ; White FG, black BG.
        .#def cursor_style 0x0f20 ; Black FG, white FG.

        .section "data"

video:
        .reserve 384
        
keyboard_index: .reserve 1
lem1802_index: .reserve 1
        
character_pos:  .reserve 1

        .section "text"

        ;; Set up the interrupt handler.
        IAS     handle_keyboard

        JSR     find_lem1802
        IFE     A, 0
        SET     PC, done
        SET     [lem1802_index], A

        JSR     find_keyboard
        IFE     A, 0
        SET     PC, done
        SET     [keyboard_index], A

        ;; Set up the monitor.
        SET     A, 0
        SET     B, video
        HWI     [lem1802_index]

        ;; Enable interrupts on the keyboard.
        SET     A, 3
        SET     B, [keyboard_index]
        HWI     [keyboard_index]

        JSR     display_cursor

done:   SET     PC, done

display_cursor:
        SET     A, [character_pos]
        SET     [A + video], cursor_style

        SET     PC, POP

;;; IN:
;;;     1 - The character to display.
display_character:
        SET     A, PICK 1       ; The character.
        SET     B, [character_pos]

        ;; Apply the backspace character
        IFN     A, 0x10
        SET     PC, _display

        ;; Check to see if we're at the beginning of the screen.
        IFE     B, 0
        SET     PC, _done

        SET     [B + video], empty_style
        SUB     [character_pos], 1
        SUB     B, 1
        SET     [B + video], font_style
        SET     PC, _done

_display:
        ;; Check to make sure it's a displayable character.
        IFL     A, 32
        SET     PC, _done
        IFG     A, 126
        SET     PC, _done
        
        SET     A, font_style
        BOR     A, PICK 1
        SET     [B + video], A
        ADD     [character_pos], 1

_done:
        JSR     display_cursor
        
        SET     PC, POP

handle_keyboard:
        SET     A, 1
        HWI     [keyboard_index]
        
        SET     PUSH, C
        JSR     display_character
        ADD     SP, 1

        ;; Clear the keyboard buffer.
        SET     A, 0
        HWI     [keyboard_index]
        
        RFI     0

        .#include "devices.asm"
