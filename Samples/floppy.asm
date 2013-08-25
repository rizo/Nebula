        .section "text"
        SET     PC, main

        .#include "devices.asm"

        .section "data"

m35fd_index:
        .reserve 1

test_data:
        .dat 1, 2, 3, 4, 5

        .section "text"
main:   


        JSR     find_m35fd
        IFE     A, 0
        SET     PC, done

        SET     [m35fd_index], A

        ;; Poll for the floppy's status.
        SET     A, 0
        HWI     [m35fd_index]

        ;; Read from the first sector of the floppy, overwritting
	;; 'test_data'.
        SET     A, 2
        SET     X, 0
        SET     Y, test_data
        HWI     [m35fd_index]

        ;; Check our work.
        SET     PUSH, [test_data]
        SET     PUSH, [test_data + 1]
        SET     PUSH, [test_data + 2]
        SET     PUSH, [test_data + 3]
        SET     PUSH, [test_data + 4]

done:
        SET     PC, done
