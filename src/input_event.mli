open Functional

open Unsigned

type t =
  | Quit
  | Key_down of uint8

val poll : t option IO.t
