include Tsdl.Sdl

module S = Tsdl.Sdl

let lift format f =
  Lwt.wrap begin fun () ->
    match f () with
    | `Error e -> prerr_endline (Format.sprintf format e); exit 1
    | `Ok v -> v
  end
