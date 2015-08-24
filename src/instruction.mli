open Prelude

open Unsigned

(** An instruction. *)
type t =
  | Binary of Code.t * Address.t * Address.t
  | Unary of Special_code.t * Address.t

(** Check if the instruction is conditional.

    Also see {! Code.conditional}. *)
val conditional : t -> bool

(** The total encoded size of the instruction in memory. *)
val encoded_size : t -> int

(** Execute the instruction. *)
val execute : t -> unit Computer_state.t
