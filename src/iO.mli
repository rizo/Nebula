(** Context for impure computations.

    This might be familiar to those who have used the IO monad in Haskell. A
    monad and functor interface are available.

    OCaml exceptions are incompatible with this interface, so {! throw} and {!
    catch} are provided for exception handling within the IO context. To lift
    impure APIs into the IO context, use {! lift}. *)

type 'a t

(** An impure computation that has failed with an exception.

    Thrown exceptions can be recovered from with {! catch}. *)
val throw : exn -> 'a t

(** Recover from a thrown exception.

    If the first argument does not result in an exception, then it is executed unchanged. *)
val catch : 'a t -> (exn -> 'a t) -> 'a t

(** Lift an impure function to the IO context.

    Native OCaml exceptions that are raised inside the body of the function are
    converted to {! IO} contexts that have had an exception thrown, as if by {!
    throw}.

    This is useful for correctly wrapping an impure API. *)
val lift : (unit -> 'a) -> 'a t

(** Lift a pure value into the IO context. *)
val unit : 'a -> 'a t

module Monad_instance : Monad_class.S with type 'a t = 'a t

module Monad : module type of Monad_class.Extend(Monad_instance)

module Functor_instance : Functor_class.S with type 'a t = 'a t

module Functor : module type of Functor_class.Extend(Functor_instance)

(** Unwrap an impure computation.

    {b This function should virtually never be used unless you know what you're
    doing.} *)
val unsafe_perform : 'a t -> 'a

(** Top-level entry-point for impure programs.

    You should combine IO computations with the functions and
    interfaces in this module and execute {! main} only at the main entry-point to
    your program. *)
val main : 'a t -> unit

(** Exit the program with a return code. *)
val terminate : int -> 'a t

(** Output a string to the standard output device. *)
val put_string : string -> unit t
