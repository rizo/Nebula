open Functional

module Inner : module type of State.Make (Dcpu16_state)

module Error : sig
  type t =
    | Already_defined_label of string
    | Undefined_labels of string list
end

include module type of Either_trans.Make (Error) (Inner.Monad_instance)

val run : ?labels:Dcpu16_state.label_map ->
  Word.t -> 'a t -> Dcpu16_state.t * (Error.t, 'a) Either.t

module Block : sig
  type t = {
    encoded : Assembled.t list;
    labels : Dcpu16_state.label_map;
  }

  val is_dependent : t -> bool

  val merge_labels : t -> t -> (Error.t, Dcpu16_state.label_map) Either.t
end

val assemble : ?at:Word.t -> ?labels:Dcpu16_state.label_map ->
  'a t ->
  (Error.t, Block.t) Either.t

val assemble_and_link : ?at:Word.t ->
  'a t ->
  (Error.t, Word.t list) Either.t

val emit : Assembled.t list -> unit t

val label : int t

val define : string -> unit t -> unit t

val assemble_value : (_, _) Value.t -> (Assembled.t * Assembled.t option) t

val assemble_inst : Inst.t -> unit t

val write_file : file_name : string -> Word.t list -> unit IO.t
