type t =
  | Quit

val poll : t option IO.t
