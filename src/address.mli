(** Addressing schemes.

    The DCPU-16 supports multiple addressing schemes (means of reading and
    writing data to registers and memory).

    An {! t} refers to a {b relative} location, which changes based on current
    state of the DCPU-16 processor and memory. Addresses are resolved to an
    absolute location via {! target_of}.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Prelude

(** An address. *)
type t =
  | Reg_direct of Reg.t
  | Reg_indirect of Reg.t
  | Reg_indirect_offset of Reg.t
  | Push
  | Pop
  | Peek
  | Pick
  | SP
  | PC
  | EX
  | Direct
  | Indirect
  | Literal of word

(** An operation was attempted on address that doesn't support it. *)
exception Invalid_operation

(** The extra encoded size of the address in memory.

    Most addresses can be encoded into the same [word] as the opcode, but some
    addresses require additional space to be encoded. *)
val extra_encoded_size : t -> int

(** Address target.

    In the context of an instruction such as

    {[
      SET X, POP
    ]}

    each address must be resolved to a {i target} exactly once within the
    instruction.

    Since a target refers to an unchanging location (unlike an {! Address}), it
    can be written to and read from an arbitrary number of times without the
    result changing. *)
module Target : sig
  type t =
    | Reg of Reg.t
    | Special of Special.t
    | Offset of word
    | Push
    | Pop
    | Value of word

  (** Read the value from the target. *)
  val get : t -> word Computer_state.t

  (** Set a value at the target. *)
  val set : word -> t -> unit Computer_state.t
end

(** Resolve a relative address to an absolute location based on the current
    computer state. *)
val target_of : t -> Target.t Computer_state.t
