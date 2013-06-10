;;; software-interrupt.asm
;;;
;;; Checks that software interrupts work correctly.

        .def loop_iterations 10

        SET     X, 0
        SET     Z, handle_interrupt
        IAS     handle_interrupt

_loop:
        IFE     I, loop_iterations
        SET     PC, _done

        INT     0x20

        ADD     I, 1
        SET     PC, _loop
        
_done:
        SET     PC, _done

handle_interrupt:
        ADD     X, 1
        RFI     0
