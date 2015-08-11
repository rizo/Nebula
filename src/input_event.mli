type t =
  | Quit

val poll: unit -> t option Lwt.t
