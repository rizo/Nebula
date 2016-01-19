(** Pure computer state.

    This structure encompasses the entire state of the DCPU-16 including the
    processor, the memory, and interrupt handling.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

type t = {
  memory : Mem.t;
  cpu : Cpu.t;
  ic : Interrupt_control.t;
  manifest : Manifest.t;
  state_error : Invalid_operation.t option;
}

(** The default computer state, with everything initially in its "empty"
    state. *)
val default : t

val set_state_error : Invalid_operation.t -> t -> t

(** Display the values of registers and where they point to in memory. *)
val show : t -> string
