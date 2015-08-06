(** The free monad. *)

(** Any {! Functor_class.S} instance is a monad "for free". *)
module Make : functor (F : Functor_class.S) -> sig
  type 'a t =
    | Return of 'a           (** A pure value. *)
    | Suspend of 'a t F.t    (** A suspended computation that needs to be executed. *)

  (** Lift a functor value into the free monad. *)
  val lift : 'a F.t -> 'a t

  module Monad_instance : (Monad_class.S with type 'a t = 'a t)

  module Monad : module type of Monad_class.Extend(Monad_instance)
end
