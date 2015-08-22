(** The SDL library for audio, video, and input events. *)

open Functional

include Tsdl.Sdl

(** Wrap an SDL function into the IO context, with an error message. *)
let lift format f =
  let module S = Tsdl.Sdl in

  IO.lift begin fun () ->
    match f () with
    | `Error e -> prerr_endline (Format.sprintf format e); exit 1
    | `Ok v -> v
  end
