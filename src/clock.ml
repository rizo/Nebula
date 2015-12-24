(** Clock interfaces and implementations.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

open Functional

module type S = sig
  val get_time : Time_stamp.t IO.t
end

module Precision = struct
  module Foreign = struct
    external get_time : unit -> int64 = "nebula_clock_precision_get_time"
  end

  let get_time =
    IO.lift Foreign.get_time
end
