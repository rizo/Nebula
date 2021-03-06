(** The free monad.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

(** Any {! Functor_class.S} instance is a monad "for free". *)
module Make : functor (F : Functor_class.S) -> sig
  type 'a t =
    | Return of 'a           (** A pure value. *)
    | Suspend of 'a t F.t    (** A suspended computation that needs to be executed. *)

  (** Lift a functor value into the free monad. *)
  val lift : 'a F.t -> 'a t

  module Monad_instance : (Monad_class.S with type 'a t = 'a t)

  module Monad : module type of Monad_class.Extend (Monad_instance)

  module Functor_instance : module type of Functor_class.Of_monad (Monad_instance)

  module Functor : module type of Functor_class.Extend (Functor_instance)
end
