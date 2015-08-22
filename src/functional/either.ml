open Prelude

type ('a, 'b) t = ('a, 'b) either

module Monad_instance(E : sig type t end)
  : (Monad_class.S with type 'a t = (E.t, 'a) either) =
struct
  type error = E.t

  type 'a t = (error, 'a) either

  let pure a =
    Right (Lazy.force a)

  let bind f = function
    | Left e -> Left e
    | Right x -> f x
end
