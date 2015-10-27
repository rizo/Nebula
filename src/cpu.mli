(** CPU registers, special registers, and flags.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

type t

(** Boolean flags dictating execution behavior. *)
module Flag : sig
  type t =
    | Skip_next (** Skip the next instruction if [true]. *)
end

(** The default initialized CPU state.

    All registers and special registers are initialized to zero. Flags are
    initialized to [false]. *)
val empty : t

val read_register : Reg.t -> t -> word

val write_register : Reg.t -> word -> t -> t

val read_special : Special.t -> t -> word

val write_special : Special.t -> word -> t -> t

val set_flag : Flag.t -> bool -> t -> t

val get_flag : Flag.t -> t -> bool
