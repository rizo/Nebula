open Prelude

module type S = sig
  type 'a monad

  type error

  type 'a t

  val run : 'a t -> (error, 'a) Either.t monad

  val unit  : 'a -> 'a t

  val lift : 'a monad -> 'a t

  val error : error -> 'a t

  val recover : (error -> 'a t) -> 'a t -> 'a t

  module Functor_instance : Functor_class.S with type 'a t = 'a t

  module Functor : module type of Functor_class.Extend (Functor_instance)

  module Monad_instance : Monad_class.S with type 'a t = 'a t

  module Monad : module type of Monad_class.Extend (Monad_instance)

  module Applicative_instance : Applicative_class.S with type 'a t = 'a t

  module Applicative : module type of Applicative_class.Extend (Applicative_instance)
end

module Make (E : sig type t end) (M : Monad_class.S)
  : (S with type 'a monad := 'a M.t
        and type error := E.t) =
struct
  module F = Functor_class.Of_monad (M)

  module Self = struct
    type 'a t = Run of (E.t, 'a) Either.t M.t
  end

  include Self

  let run (Run inner) =
    inner

  module Monad_instance = struct
    type 'a t = 'a Self.t

    let pure a =
      Run (M.pure (lazy (Right (Lazy.force a))))

    let bind f ma =
      Run begin
        M.bind
          (function
          | (Left e as le) -> M.pure (lazy le)
          | Right r -> run (f r))
          (run ma)
      end
  end

  let unit a =
    Monad_instance.pure (lazy a)

  module Monad = Monad_class.Extend (Monad_instance)

  module Functor_instance = struct
    module Em = Either.Monad_instance (E)
    module Ef = Functor_class.Of_monad (Em)

    type 'a t = 'a Self.t

    let map f t =
      Run (F.map (Ef.map f) (run t))
  end

  module Functor = Functor_class.Extend (Functor_instance)

  let lift ma =
    Run (F.map (fun a -> Right a) ma)

  let error e =
    Run (M.pure (lazy (Left e)))

  let recover f t =
    Run begin
      M.bind
        (function
          | Left e -> run (f e)
          | Right a -> M.pure (lazy (Right a)))
        (run t)
    end

  module Applicative_instance = Applicative_class.Of_monad (Monad_instance)

  module Applicative = Applicative_class.Extend (Applicative_instance)
end
