;;; monitor-font.asm
;;;
;;; Demonstrate the font built-in to the LEM1802 display.

;;;
;;; Constants
        .def font_style 0xf000  ; White FG, black BG.
        .def num_characters 128
        
        .section "data"
video:
        .reserve 384

lem1802_index:
        .reserve 1
        
;;; 
;;; Program
        .section "text"

        JSR     find_lem1802
        IFE     A, 0
        SET     PC, done        ; Failed to find the LEM1802.
        SET     [lem1802_index], A
        
        JSR     display
done:   
        SET     PC, done

display:
        ;; Format the message in memory.
        SET     I, 0            ; Character index.
_loop:
        IFE     I, num_characters
        SET     PC, _done

        SET     Z, font_style
        BOR     Z, I
        SET     [video + I], Z

        ADD     I, 1
        SET     PC, _loop

_done:
        ;; Set video RAM address.
        SET     A, 0
        SET     B, video
        HWI     [lem1802_index]
        SET     PC, POP

        .include "devices.asm"
