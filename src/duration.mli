(** Finite duration of time.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

type t

(** Express a duration in terms of elapsed nanoseconds.

    Negative integral values are interpretted as zero-length durations. *)
val of_nanoseconds : int64 -> t

(** Number of nanoseconds in the duration. *)
val nanoseconds : t -> int64

(** Add two durations. *)
val (+) : t -> t -> t

(** Subtract two durations.

    If the second duration is larger than the first, a zero-length duration is
    the result. *)
val (-) : t -> t -> t

val (=) : t -> t -> bool

val (<>) : t -> t -> bool

val (<) : t -> t -> bool

val (<=) : t -> t -> bool

val (>) : t -> t -> bool

val (>=) : t -> t -> bool
