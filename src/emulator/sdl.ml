(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Functional

include Tsdl.Sdl

let lift format f =
  let module S = Tsdl.Sdl in

  IO.lift begin fun () ->
    match f () with
    | Result.Error (`Msg e) -> prerr_endline (Format.sprintf format e); exit 1
    | Result.Ok v -> v
  end
