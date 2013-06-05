;;; find-device.asm
;;;
;;; IN:
;;;     3 - First word of the device ID.
;;;     2 - Second word of the device ID.
;;;     1 - Device revision.
;;; 
;;; OUT:
;;;     X - The hardware index. This is 0xffff if the device cannot
;;;         be found.
find_device:
        SET     PUSH, A
        SET     PUSH, B
        SET     PUSH, C
        SET     PUSH, I
        SET     PUSH, J
        
        HWN     I
        SET     J, 0
        
_loop:
        HWQ     J
        
        ;; Check the current device against the desired device.
        IFE     A, PICK 8
        IFE     B, PICK 7
        IFE     C, PICK 6
        SET     PC, _success

        ;; Not this device.
        ADD     J, 1
        
        IFE     J, I            ; Device not found.
        SET     PC, _failure

        SET     PC, _loop
        
_success:
        SET     X, J
        SET     PC, _done
_failure:
        SET     X, 0xffff
_done:
        SET     J, POP
        SET     I, POP
        SET     C, POP
        SET     B, POP
        SET     A, POP
        
        SET     PC, POP
        
        
