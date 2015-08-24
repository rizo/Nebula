(** Decode binary data for the processor.

    Generally, decoding errors will result in a value of {! None}. *)

open Functional
open Prelude

(** The address context.

    In an instruction like

    {[
      ADD X, [Y]
    ]}

    the [A] context is [[Y]] and the [B] context is [X]. *)
type context = A | B

(** Decode an opcode. *)
val code : word -> Code.t option

(** Decode a special opcode. *)
val special_code : word -> Special_code.t option

(** Decode a register. *)
val register : word -> Reg.t option

(** Decode an address. *)
val address : context -> word -> Address.t option

(** Decode an instruction. *)
val instruction : word -> Instruction.t option
