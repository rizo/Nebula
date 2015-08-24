(** Simulated hardware registers. *)

type t =
  | A | B | C
  | X | Y | Z
  | I | J
[@@deriving eq, ord, show]
