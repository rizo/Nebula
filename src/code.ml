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
  | Ifg
  | Ifl
  | Sti

let conditional = function
  | Ife
  | Ifn
  | Ifg
  | Ifl -> true
  | _ -> false
