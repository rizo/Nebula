        .org 0x50

keyboard_index:
        .reserve 1

        .include "find-device.asm"

        .org 0

        IAS     handle_keyboard

        ;; Find the keyboard
        SET     PUSH, 0x7406
        SET     PUSH, 0x30cf
        SET     PUSH, 1
        JSR     find_device
        SET     0, POP
        SET     0, POP
        SET     0, POP

        IFE     X, 0
        SET     PC, done
        SET     [keyboard_index], X

        ;; Enable interrupts on the keyboard.
        SET     A, 3
        SET     B, 0xbeef
        HWI     [keyboard_index]

done:   SET     PC, done

handle_keyboard:
        ;; Store the key in X.
        SET     A, 1
        HWI     [keyboard_index]
        SET     X, C

        ;; Clear the keyboard buffer.
        SET     A, 0
        HWI     [keyboard_index]
        
        RFI     0

