open Functional

module Inner : module type of State.Make (Dcpu16_state)

module Error : sig
  type t = Undefined_label of string
end

include module type of Either_trans.Make (Error) (Inner.Monad_instance)

val run : Word.t -> 'a t -> Dcpu16_state.t * (Error.t, 'a) Either.t

val assemble : ?at:Word.t -> unit t -> (Error.t, Word.t list) Either.t

val emit : Word.t list -> unit t

val label : Value.address t
