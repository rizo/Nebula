(** Interrupts. *)

open Prelude

(** The index of a hardware device. *)
type index = word

type t = Message of word

(** Interrupts can be triggered from hardware devices or directly from the
    processor. *)
module Trigger = struct
  type t =
    | Software of word
    | Hardware of index
end

