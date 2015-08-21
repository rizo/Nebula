(** Operation codes ("opcodes") for DCPU-16 instructions. *)

type t =
  | Set
  | Add
  | Sub
  | Mul
  | Mli
  | Div
  | Dvi
  | Bor
  | Shl
  | Ife
  | Ifn
  | Ifl

let conditional = function
  | Ife -> true
  | _ -> false
