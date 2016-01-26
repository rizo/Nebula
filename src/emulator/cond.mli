(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

type t

type outcome =
  | Hit of string
  | Miss

val check : Computer_state.t -> t -> outcome * t option

val break : Word.t -> t

val continue : t

val step : int -> t
