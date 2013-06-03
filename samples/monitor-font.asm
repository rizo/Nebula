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

lem1802_index: .reserve 1

;;; 
;;; Program

        .org 0x0

        JSR     find_lem1802
        JSR     display
done:   
        SET     PC, done

find_lem1802:
        HWN     I               ; Get the number of connected devices.
        SET     J, 0            ; Index counter.
_loop:
        HWQ     J
        ;; Check the current device against the ID of the LEM1802.
        IFE     A, 0xf615
        IFE     B, 0x7349
        IFE     C, 0x1802
        SET     PC, _success

        ;; Not this device. Increment.
        ADD     J, 1
        IFE     J, I
        SET     PC, _failure    ; LEM1802 not found.

        SET     PC, _loop
_success:
        SET     [lem1802_index], J
        SET     PC, POP
_failure:
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
        ;; Set the border color.
        SET     B, 3
        SET     A, 3
        HWI     [lem1802_index]
        
        ;; Set video RAM address.
        SET     A, 0
        SET     B, video_addr
        HWI     [lem1802_index]
        SET     PC, POP
