(** Operation codes ("opcodes") for DCPU-16 instructions. *)

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

let conditional = function
  | Ifb
  | Ifc
  | Ife
  | Ifn
  | Ifg
  | Ifa
  | Ifl
  | Ifu -> true
  | _ -> false
