(** Simulated special hardware registers used for program flow. *)

type t = SP | PC | EX | IA
[@@deriving eq, ord, show]
