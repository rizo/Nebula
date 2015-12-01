(** {! Computer} operations interpretted in the context of the {! State} monad.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Functional

include module type of State.Make(Computer_state)

(** Interpret a {! Program} in the {! Computer} context. *)
val of_program : 'a Program.t -> 'a t
