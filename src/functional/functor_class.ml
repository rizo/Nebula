(** The functor abstraction. *)

module type S = sig
  type _ t

  val map : ('a -> 'b) -> 'a t -> 'b t
end

module Of_monad(M : Monad_class.S) : (S with type 'a t = 'a M.t) = struct
  type 'a t = 'a M.t

  let map f ma =
    M.bind (fun a -> M.pure (lazy (f a))) ma
end

module type EXTENSION = sig
  type _ t

  include S with type 'a t := 'a t

  val ( <$> ) : ('a -> 'b) -> 'a t -> 'b t
end

module Extend(F : S) : (EXTENSION with type 'a t := 'a F.t) = struct
  include F

  let ( <$> ) f ma =
    F.map f ma
end
