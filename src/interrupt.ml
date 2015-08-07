(** Interrupts. *)

open Prelude

type t = Message of word

(** Interrupts can be triggered from hardware devices or directly from the
    processor. *)
module Trigger = struct
  type t =
    | Software of message
    | Hardware of device_index
end

