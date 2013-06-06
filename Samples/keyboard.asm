;;; keyboard.asm
;;;
;;; Basic keyboard demo which writes characters to the screen.

;;;
;;; Constants

        .def video_addr 0x9000
        .def data_addr 0x100
        .def font_style 0xf000           ; White FG, black BG.
        
        .org data_addr

keyboard_index: .reserve 1
lem1802_index: .reserve 1
character_pos:  .reserve 1

        .include "devices.asm"

        .org 0

        ;; Set up the interrupt handler.
        IAS     handle_keyboard

        JSR     find_lem1802
        IFE     X, 0
        SET     PC, done
        SET     [lem1802_index], X

        JSR     find_keyboard
        IFE     X, 0
        SET     PC, done
        SET     [keyboard_index], X

        ;; Set up the monitor.
        SET     A, 0
        SET     B, video_addr
        HWI     [lem1802_index]

        ;; Enable interrupts on the keyboard.
        SET     A, 3
        SET     B, [keyboard_index]
        HWI     [keyboard_index]

done:   SET     PC, done        

;;; IN:
;;;     1 - The character to display.
;;;
;;; OUT:
display_character:
        SET     PUSH, Y

        SET     X, PICK 2       ; The character.
        SET     Y, [character_pos]

        ;; Apply the backspace character
        IFN     X, 0x10
        SET     PC, _display

        ;; Check to see if we're at the beginning of the screen.
        IFE     Y, 0
        SET     PC, _done

        SUB     [character_pos], 1
        SUB     Y, 1
        SET     [video_addr + Y], font_style
        SET     PC, _done
        
_display:
        ;; Check to make sure it's a displayable character.
        IFL     X, 32
        SET     PC, _done
        IFG     X, 126
        SET     PC, _done
        
        SET     X, font_style
        BOR     X, PICK 2
        SET     [video_addr + Y], X
        ADD     [character_pos], 1

_done:        
        SET     Y, POP
        
        SET     PC, POP

handle_keyboard:
        SET     A, 1
        HWI     [keyboard_index]
        
        SET     PUSH, C
        JSR     display_character
        SET     0, POP

        ;; Clear the keyboard buffer.
        SET     A, 0
        HWI     [keyboard_index]
        
        RFI     0
