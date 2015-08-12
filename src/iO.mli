type 'a t

val lift : (unit -> 'a) -> 'a t

val unit : 'a -> 'a t

module Monad_instance : Monad_class.S with type 'a t = 'a t

module Monad : module type of Monad_class.Extend(Monad_instance)

module Functor_instance : Functor_class.S with type 'a t = 'a t

module Functor : module type of Functor_class.Extend(Functor_instance)

val unsafe_perform : 'a t -> 'a

val main : 'a t -> unit

val terminate : int -> 'a t

val put_string : string -> unit t
