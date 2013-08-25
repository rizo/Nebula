;;; floppy.asm
;;;
;;; Test out the floppy drive.

        .section "text"
        SET     PC, main

        .#include "devices.asm"

        .section "data"

m35fd_index:
        .reserve 1

m35fd_ready:
        .reserve 1

memory_data:
        .dat 1, 2, 3, 4, 5
        .reserve 507

floppy_data:    
        .dat 6, 7, 8, 9, 10

        .section "text"
main:
        JSR     find_m35fd
        IFE     A, 0
        SET     PC, done

        SET     [m35fd_index], A

        IAS     interrupt_handler
        SET     A, 1
        SET     X, 0x35
        HWI     [m35fd_index]

        ;; Write to the first sector of the floppy.
        SET     A, 3
        SET     X, 0
        SET     Y, floppy_data
        HWI     [m35fd_index]

        JSR     wait_for_m35fd

        ;; Read from the first sector of the floppy.
        SET     A, 2
        SET     X, 0
        SET     Y, memory_data
        HWI     [m35fd_index]

        JSR     wait_for_m35fd

        ;; Check our work.
        ;; The stack should match 'floppy_data'.
        SET     PUSH, [memory_data + 4]
        SET     PUSH, [memory_data + 3]
        SET     PUSH, [memory_data + 2]
        SET     PUSH, [memory_data + 1]
        SET     PUSH, [memory_data]

done:
        SET     PC, done

wait_for_m35fd:
        SET     [m35fd_ready], 0
_loop:
        IFN     [m35fd_ready], 0
        SET     PC, _ready
        SET     PC, _loop
_ready:  
        SET     PC, POP

interrupt_handler:
        SET     [m35fd_ready], 1
        RFI     0
