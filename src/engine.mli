(** Manage the running DCPU-16 computer. *)

open Functional
open Prelude

(** A {! word} could not be decoded by the DCPU-16. *)
exception Bad_decoding of word * Computer.t

(** A device was requested at an index that has not been assigned. *)
exception No_such_device of word * Computer.t

(** Jump to the interrupt handler and disable interrupt dequeing. *)
val handle_interrupt : Interrupt.t -> Computer.t -> Computer.t

(** Execute a triggered interrupt.

    Software interrupts are enqueued directly. Hardware interrupts cause the
    interrupt hook for the device registered at the interrupt index to be
    invoked. *)
val execute_interrupt : Interrupt.Trigger.t -> Computer.t -> Computer.t IO.t

(** Execute an iteration of the DCPU-16.

    First, dequeue an interrupt (if one is waiting) and transfer control to the interrupt handler.

    Next, decode the next instruction from memory at the program counter.

    Finally, check for triggered interrupts (either by software, or by a device
    when it was "ticked") and execute them. *)
val step : Computer.t -> Computer.t IO.t

(** "Tick" all devices in the manifest. *)
val tick_devices : Computer.t -> Computer.t IO.t

(**  Launch a DCPU-16.

     Control is suspended every [suspend_every] nanoseconds to [suspension]. Otherwise, this
     computation will never terminate unless it throws. *)
val launch :
  suspend_every : int ->
  suspension : (Computer.t -> Computer.t IO.t) ->
  Computer.t ->
  'a IO.t
