(** Specialization of {! Either} for error handling with exceptions.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Prelude

type 'a t = (exn, 'a) either

(** Converts a function that raises exceptions to one where error is indicated
    in the return value. *)
let protect f =
  fun a ->
    try Right (f a) with
    | exn -> Left exn

module Monad_instance = Either.Monad_instance (Exception)

module Monad = Monad_class.Extend (Monad_instance)

module Functor_instance = Functor_class.Of_monad (Monad_instance)

module Functor = Functor_class.Extend (Functor_instance)
