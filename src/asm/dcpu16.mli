open Functional

module Inner : module type of State.Make (Dcpu16_state)

module Error : sig
  type t =
    | Already_defined_label of string
end

include module type of Either_trans.Make (Error) (Inner.Monad_instance)

val run : Word.t -> 'a t -> Dcpu16_state.t * (Error.t, 'a) Either.t

val assemble : ?at:Word.t ->
  'a t ->
  (Error.t, Assembled.t list * (string * Label_loc.t) list) Either.t

val emit : Assembled.t list -> unit t

val label : Value.address t

val define : string -> unit t -> unit t

val loc : string -> (Value.direct, Word.t) Value.t t
