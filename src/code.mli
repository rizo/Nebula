(** Operation codes ("opcodes") for the DCPU-16 instructions. *)

type t =
  | Set
  | Add
  | Sub
  | Mul
  | Mli
  | Div
  | Dvi
  | Mod
  | Mdi
  | And
  | Bor
  | Xor
  | Shr
  | Asr
  | Shl
  | Ifb
  | Ifc
  | Ife
  | Ifn
  | Ifg
  | Ifa
  | Ifl
  | Ifu
  | Adx
  | Sbx
  | Sti
  | Std

(** Whether the opcode refers to a conditional instruction. *)
val conditional : t -> bool
