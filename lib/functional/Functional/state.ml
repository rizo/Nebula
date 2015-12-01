(** The state monad.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

module type S = sig
  type _ t

  (** The state being managed. *)
  type state

  module Monad_instance : Monad_class.S with type 'a t = 'a t

  module Monad : module type of Monad_class.Extend (Monad_instance)

  module Functor_instance : module type of Functor_class.Of_monad (Monad_instance)

  module Functor : module type of Functor_class.Extend (Functor_instance)

  (** Lift a function returning a result and a new state. *)
  val lift : (state -> state * 'a) -> 'a t

  (** Run the computation producing a new state and a value. *)
  val run : state -> 'a t -> state * 'a

  (** Retrieve the state. *)
  val get : state t

  (** Retreive the state and apply an accessor to it. *)
  val gets : (state -> 'a) -> 'a t

  (** Set a new value for the state. *)
  val set : state -> unit t

  (** Modify the state. *)
  val modify : (state -> state) -> unit t

  (** Wrap a pure value in the state context. *)
  val unit : 'a -> 'a t
end

(** Parameterize {! S} on a specific state type [K.t]. *)
module Make (K : sig type t end) : (S with type state := K.t) = struct
  module Run = struct
    type 'a t = K.t -> K.t * 'a
  end

  type 'a t = 'a Run.t

  let lift f:(K.t -> K.t * 'a) =
    f

  let run k ma =
    ma k

  module Monad_instance = struct
    type 'a t = 'a Run.t

    let pure a =
      fun k -> (k, Lazy.force a)

    let bind f ma =
      fun k ->
        let (k2, a) = ma k in
        (f a) k2
  end

  module Monad = Monad_class.Extend (Monad_instance)

  module Functor_instance = Functor_class.Of_monad (Monad_instance)

  module Functor = Functor_class.Extend (Functor_instance)

  let get =
    fun k -> (k, k)

  let gets f =
    let open Functor in
    f <$> get

  let set k =
    fun _ -> (k, ())

  let modify f =
    Monad.(get >>= fun k -> set (f k))

  let unit a =
    Monad.pure (lazy a)
end
