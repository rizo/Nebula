(** Fundemental definitions. *)

(** Convenient alias for {! Word.t } *)
type word = Word.t

(** Convenient alias for {! Word.of_int }. *)
val word : int -> word

(** Simulated devices are each given a unique index. *)
type device_index = word

(** The contents of an interrupt. *)
type message = word

(** Function composition. *)
val ( @ ) : ('b -> 'c) -> ('a -> 'b) -> 'a -> 'c

(** Constant function. *)
val const : 'a -> 'b -> 'a

(** Identity function. *)
val id : 'a -> 'a

type ('a, 'b) either =
  | Left of 'a
  | Right of 'b

module Int : sig
  type t = int

  val compare : t -> t -> int
end

module Float : sig
  type t = float

  val compare : t -> t -> int
end

module Exception : sig
  type t = exn
end
