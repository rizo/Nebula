(** Concurrent execution in the {! IO} context.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

include Thread.S with type 'a context := 'a IO.t
