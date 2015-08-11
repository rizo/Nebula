type t =
  | Quit

let poll () =
  Lwt.wrap begin fun () ->
    let event = Sdl.Event.create () in
    if Sdl.poll_event (Some event) then
      let typ = Sdl.Event.(get event typ) in
      if typ = Sdl.Event.quit then Some Quit
      else None
    else None
  end
