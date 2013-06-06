;;; monitor-font.asm
;;;
;;; Demonstrate the font built-in to the LEM1802 display.

;;;
;;; Constants

        .def video_addr 0x9000
        .def data_addr 0x50
        .def font_style 0xf000  ; White FG, black BG.
        .def num_characters 128
        
;;; 
;;; Data

        .org data_addr

lem1802_index:
        .reserve 1
        
        .include "devices.asm"
;;; 
;;; Program

        .org 0x0

        JSR     find_lem1802
        IFE     X, 0
        SET     PC, done        ; Failed to find the LEM1802.
        SET     [lem1802_index], X
        
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
        SET     [video_addr + I], Z

        ADD     I, 1
        SET     PC, _loop

_done:
        ;; Set video RAM address.
        SET     A, 0
        SET     B, video_addr
        HWI     [lem1802_index]
        SET     PC, POP
