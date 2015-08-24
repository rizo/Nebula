open Functional

include Tsdl.Sdl

let lift format f =
  let module S = Tsdl.Sdl in

  IO.lift begin fun () ->
    match f () with
    | `Error e -> prerr_endline (Format.sprintf format e); exit 1
    | `Ok v -> v
  end
