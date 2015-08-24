(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

module Make(F : Functor_class.S) = struct
  module Self = struct
    type 'a t =
      | Return of 'a
      | Suspend of 'a t F.t
  end

  include Self

  open Self

  let lift op =
    Suspend (F.map (fun x -> Return x) op)

  module Monad_instance = struct
    type 'a t = 'a Self.t

    let pure a =
      Return (Lazy.force a)

    let rec bind f = function
      | Return x -> f x
      | Suspend x -> Suspend (F.map (bind f) x)
  end

  module Monad = Monad_class.Extend(Monad_instance)

  module Functor_instance = Functor_class.Of_monad(Monad_instance)

  module Functor = Functor_class.Extend(Functor_instance)
end
