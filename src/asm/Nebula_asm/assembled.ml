(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

type t =
  | Constant of Word.t
  | Dependent of string
