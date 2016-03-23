(** Input events from the user.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Functional

open Unsigned

(** The different events than can occur. *)
type t =
  | Quit (** The emulator should terminate. *)
  | Key_down of uint8 (** The user typed a character on their keyboard. *)


(** Query for an input event.

    This function will return immediately with an event waiting to be processed,
    or [None] if no event is waiting. *)
val poll : t option IO.t
