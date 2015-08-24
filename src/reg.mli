(** Simulated hardware registers.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

type t =
  | A | B | C
  | X | Y | Z
  | I | J
[@@deriving eq, ord, show]
