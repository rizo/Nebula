(** Impure computations that involve input and/or output from the outside
    world.

    The primary interface for this type is the monadic one.

    You should virtually never leave the context of IO, unless it's in the
    context of a test. Instead, use {! main} at the main entry point of your
    program to execute any IO computations. *)

(** An impure computation that results in a value of type ['a]. *)
type 'a t

(** {2 Instances} *)

module Monad_instance : Monad_class.S with type 'a t = 'a t

module Monad : module type of Monad_class.Extend(Monad_instance)

(** {2 Basic input and output} *)

val put_string : string -> unit t

val get_string : string t

(** {2 Wrapping computations} *)

(** Wrap a thunk into an IO context.

    This can be useful for wrapping impure APIs. *)
val lift : (unit -> 'a) -> 'a t

(** Wrap a pure value into an IO context. *)
val unit : 'a -> 'a t

(** {2 Leaving the IO context} *)

(** Unwrap an IO context to a pure value.

    You should {b rarely} use this function outside of unit tests. *)
val unsafe_perform : 'a t -> 'a

(** Top-level entry-point for executing IO in a program.

    The correct way to use this module is to build a large {! IO} computation
    and then execute it at the top-level entry point.

    For instance,

    {[
      let f = IO.put_string "Hello, world!\n"

      let () =
        IO.main f
    ]} *)
val main : 'a t -> unit

(** {2 Misc. system functions} *)

(** Terminate execute of the program with the designated error code. *)
val terminate : int -> unit t
