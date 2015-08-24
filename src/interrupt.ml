(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Prelude

type t = Message of word

module Trigger = struct
  type t =
    | Software of message
    | Hardware of device_index
end

