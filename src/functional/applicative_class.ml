(** Applicative functors.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Prelude

module type S = sig
  type _ t

  val pure : 'a Lazy.t -> 'a t

  val ap : ('a -> 'b) t -> 'a t -> 'b t
end

module type EXTENSION = sig
  type _ t

  include S with type 'a t := 'a t

  val ( <*> ) : ('a -> 'b) t -> 'a t -> 'b t

  val ( <* ) : 'a t -> 'b t -> 'a t

  val ( *> ) : 'a t -> 'b t -> 'b t

  val lift2 : ('a -> 'b -> 'c) -> 'a t -> 'b t -> 'c t
end

module To_functor(F : S) : (Functor_class.S with type 'a t = 'a F.t) = struct
  type 'a t = 'a F.t

  let map f ma =
    F.ap (F.pure (lazy f)) ma
end

module Of_monad(M : Monad_class.S) : (S with type 'a t = 'a M.t) = struct
  type 'a t = 'a M.t

  let pure =
    M.pure

  module Monad = Monad_class.Extend(M)

  let ap fab fa =
    let open Monad in
    fab >>= fun f ->
    fa >>= fun a ->
    M.pure (lazy (f a))
end

module Extend(F : S) : (EXTENSION with type 'a t := 'a F.t) = struct
  include F

  module Functor = To_functor(F)

  let ( <*> ) fab fa =
    F.ap fab fa

  let lift2 f fa fb =
    (Functor.map f fa) <*> fb

  let ( <* ) fa fb =
    lift2 const fa fb

  let ( *> ) fa fb =
    ((Functor.map @ const) id) fa <*> fb
end
