(** Property-based testing libraries.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Functional

module type RANDOM_ENGINE = sig
  (** State of the random engine. *)
  type t

  (** Generate a random {! int64 } and the updated state. *)
  val next_int64 : t -> t * int64
end

(** Default {! RANDOM_ENGINE } implementation. *)
module Default_random_engine : sig
  type t

  val next_int64 : t -> t * int64

  (** Seed a new engine. *)
  val with_seed : int -> t
end

module Gen : sig
  module type S = sig
    type 'a t

    type engine

    val sample : engine -> 'a t -> engine * 'a

    val unit : 'a -> 'a t

    val int : int t

    val non_negative_int : int t

    val choose_int : low:int -> high:int -> int t

    val bool : bool t

    val pair : 'a t -> ('a * 'a) t

    val list_of_n : int -> 'a t -> 'a list t

    val list_of : 'a t -> (int -> 'a list t)

    val union : 'a t -> 'a t -> 'a t

    val choice : 'a t list -> 'a t option

    val where : ('a -> bool) -> 'a t -> 'a t

    module Functor_instance : Functor_class.S with type 'a t = 'a t

    module Functor : module type of Functor_class.Extend (Functor_instance)

    module Monad_instance : Monad_class.S with type 'a t = 'a t

    module Monad : module type of Monad_class.Extend (Monad_instance)
  end

  module Make : functor (R : RANDOM_ENGINE) -> S with type engine = R.t
end

type failed_case = string

type test_cases = int

type success_count = int

type max_size = int

module Result : sig
  type t =
    | Falsified of failed_case option * success_count
    | Proved
    | Passed of success_count

  val falsified : t -> bool
end

module Prop : sig
  module type S = sig
    type t

    type engine

    type 'a generator

    val pass : t

    val proved : t

    val ( && ) : t -> t -> t

    val check : ?label:string -> (unit -> bool) -> t

    val for_all : ?label:string
      -> 'a generator
      -> ('a -> bool)
      -> t

    val for_all_sizes : ?label:string
      -> (int -> 'a generator)
      -> ('a -> bool)
      -> t

    val run : ?test_cases:test_cases -> ?max_size:max_size -> engine -> t -> Result.t
  end

  module Make : functor (G : Gen.S)
    -> (S with type engine = G.engine
           and type 'a generator = 'a G.t)
end

module Simple_gen : sig
  include module type of Gen.Make (Default_random_engine)

  val sample_io : 'a t -> 'a IO.t
end

module Simple_prop : sig
  include module type of Prop.Make (Simple_gen)

  val run_io : ?test_cases:test_cases -> ?max_size:max_size -> t -> Result.t IO.t
end

module Suite : sig
  type t =
    | Single of Simple_prop.t
    | Group of string * t list

  val to_props : t -> Simple_prop.t list
end

module Dsl : sig
  module Engine = Default_random_engine

  module Gen = Simple_gen

  module Prop = Simple_prop

  module Suite = Suite

  val check : ?label:string -> (unit -> bool) -> Suite.t

  val for_all : ?label:string -> 'a Gen.t -> ('a -> bool) -> Suite.t

  val for_all_sizes : ?label:string -> (int -> 'a Gen.t) -> ('a -> bool) -> Suite.t

  val group : string -> Suite.t list -> Suite.t

  val run : ?test_cases:int -> ?max_size:int -> Suite.t -> unit IO.t
end

module Examples : sig
  val int : Suite.t
end
