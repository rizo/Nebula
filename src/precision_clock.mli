(** System clock with a finer glanularity than the default clock. *)

open Functional

(** Get a precision time-stamp, expressed in nanoseconds.

    The meaning of the value of this time-stamp is platform dependent, so its
    primary use is to compute the duration {b between} invocations. *)
val get_time : int IO.t
