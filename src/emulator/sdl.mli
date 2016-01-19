(** The SDL library for audio, video, and input events.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Functional

include module type of Tsdl.Sdl

(** Wrap an SDL function into the IO context, with an error message. *)
val lift :
  ('a -> string, unit, string) format ->
  (unit -> ('b, [< `Msg of 'a]) Result.result) ->
  'b IO.t
