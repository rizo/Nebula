(** Simulated special hardware registers used for program flow.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

type t = SP | PC | EX | IA
[@@deriving eq, ord, show]
