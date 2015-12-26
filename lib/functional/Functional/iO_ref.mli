(** Like OCaml refs, but in the IO context.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

type _ t

val make : 'a -> 'a t IO.t

val set : 'a -> 'a t -> unit IO.t

val get : 'a t -> 'a IO.t

val modify : ('a -> 'a) -> 'a t -> unit IO.t
