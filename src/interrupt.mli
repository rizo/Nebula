(** Hardware and software interrupts. *)

open Prelude

(** An interrupt always includes a {i message} consisting of a single {!
    word}. *)
type t = Message of message

(** Interrupts are can be triggered directly from software with a defined
    message or sent to hardware based on the device's assigned index. *)
module Trigger : sig
  type t =
    | Software of message
    | Hardware of device_index
end

