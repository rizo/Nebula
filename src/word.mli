(** The native word of the DCPU-16. It is 16 bits (2 bytes).

    This is a two's complement unsigned representation. That is, [0xffff + 1]
    will yield [0].
*)

type t

val ( + ) : t -> t -> t

val ( - ) : t -> t -> t

val ( * ) : t -> t -> t

val ( land ) : t -> t -> t

val ( lor ) : t -> t -> t

val ( lsl ) : t -> int -> t

val ( lsr ) : t -> int -> t

(** Convert from an {! int }.

    Anything beyond the first 16 lower-order bits will be discarded. *)
val of_int : int -> t

val to_int : t -> int

val to_bool : t -> bool

val compare : t -> t -> int

(** Format the word as a string.

    The format of the output is a four-digit hexadecimal string.

    For instance, [show (word 3)] will yield ["0x0003"]. *)
val show : t -> string

val pp : Format.formatter -> t -> unit
