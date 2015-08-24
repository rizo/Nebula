(** Disjunction type. Either a result or an error value. *)

open Prelude

type ('a, 'b) t = ('a, 'b) either

(** Computations which fail short-circuit execution. *)
module Monad_instance (E : sig type t end)
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
