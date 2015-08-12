open Prelude

type 'a t = (exn, 'a) either

let protect f =
  fun a ->
    try Right (f a) with
    | exn -> Left exn

module Monad_instance = Either.Monad_instance(Exception)

module Monad = Monad_class.Extend(Monad_instance)

module Functor_instance = Functor_class.Of_monad(Monad_instance)

module Functor = Functor_class.Extend(Functor_instance)
