(** The SDL library for audio, video, and input events.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Functional

include module type of Tsdl.Sdl

(** Wrap an SDL function into the IO context, with an error message. *)
val lift :
  ('a -> string, unit, string) format ->
  (unit -> [< `Error of 'a | `Ok of 'b]) ->
  'b IO.t
