(** Fundemental definitions

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

(** Function composition. *)
val ( @ ) : ('b -> 'c) -> ('a -> 'b) -> 'a -> 'c

(** Constant function. *)
val const : 'a -> 'b -> 'a

(** Identity function. *)
val id : 'a -> 'a

val list_of_options : 'a option list -> 'a list

(** Populate a {! list} from a lower bound (inclusive) to an upper bound
    (exclusive). *)
val enum_from_to : int -> int -> int list

val partition_map : ('a -> 'b option) -> 'a list -> 'b list * 'a list

val filter_map : ('a -> 'b option) -> 'a list -> 'b list

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
