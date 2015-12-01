(** Helpful functions for the {! option} type.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

type 'a t = 'a option

module Monad_instance = struct
  type 'a t = 'a option

  let pure (lazy a) =
    Some a

  let bind f = function
    | None -> None
    | Some a -> f a
end

module Monad = Monad_class.Extend (Monad_instance)

module Applicative_instance = Applicative_class.Of_monad (Monad_instance)

module Applicative = Applicative_class.Extend (Applicative_instance)

module Functor_instance = Functor_class.Of_monad (Monad_instance)

module Functor = Functor_class.Extend (Functor_instance)

let get_or_else default = function
  | Some x -> x
  | None -> Lazy.force default
