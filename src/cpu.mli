(** The CPU state. *)

open Prelude

type t

module Flag : sig
  type t =
    | Skip_next
end

(** The default initialized state.

    Registers are initialized to zero. Most special values are initialized to
    zero as well, except the stack pointer [SP], which is initialized to
    [0xffff]. *)
val empty : t

val read_register : Reg.t -> t -> word

val write_register : Reg.t -> word -> t -> t

val read_special : Special.t -> t -> word

val write_special : Special.t -> word -> t -> t

val set_flag : Flag.t -> bool -> t -> t

val get_flag : Flag.t -> t -> bool
