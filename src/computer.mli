(** Pure computer state.

    This structure encompasses the entire state of the DCPU-16 including the
    processor, the memory, and interrupt handling. *)

open Prelude

type t = {
  memory : Mem.t;
  cpu : Cpu.t;
  interrupt_ctrl : Interrupt_control.t;
  manifest : Manifest.t;
}

(** The default computer state, with everything initially in its "empty"
    state. *)
val default : t

(** Display the values of registers and where they point to in memory. *)
val show : t -> string
