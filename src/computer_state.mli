(** {! Computer} operations interpretted in the context of the {! State} monad.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Functional
open Prelude

include module type of State.Make(Computer)

(** Interpret a {! Program} in the {! Computer_state} context. *)
val of_program : 'a Program.t -> 'a t
