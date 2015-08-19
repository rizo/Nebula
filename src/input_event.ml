open Unsigned

type t =
  | Quit
  | Key_down of uint8

let code_of_keycode = function
  | n when n = Sdl.K.backspace -> Some (UInt8.of_int 0x10)
  | n when n = Sdl.K.return -> Some (UInt8.of_int 0x11)
  | n when n = Sdl.K.insert -> Some (UInt8.of_int 0x12)
  | n when n = Sdl.K.delete -> Some (UInt8.of_int 0x13)
  | n when n = Sdl.K.up -> Some (UInt8.of_int 0x80)
  | n when n = Sdl.K.down -> Some (UInt8.of_int 0x81)
  | n when n = Sdl.K.left -> Some (UInt8.of_int 0x82)
  | n when n = Sdl.K.right -> Some (UInt8.of_int 0x83)
  | n when n = Sdl.K.lshift || n = Sdl.K.rshift -> Some (UInt8.of_int 0x90)
  | n when n = Sdl.K.lctrl || n = Sdl.K.rctrl -> Some (UInt8.of_int 0x91)
  | _ -> None

let poll =
  IO.lift begin fun () ->
    let event = Sdl.Event.create () in

    if Sdl.poll_event (Some event) then
      let typ = Sdl.Event.(get event typ) in

      if typ = Sdl.Event.quit then Some Quit
      else if typ = Sdl.Event.key_down then begin
        let keycode = Sdl.Event.(get event keyboard_keycode) in
        keycode |> code_of_keycode |> Option.Functor.map (fun c -> Key_down c)
      end
      else if typ = Sdl.Event.text_input then begin
        let text = Sdl.Event.(get event text_input_text) in
        if String.length text = 1 then
          let ch = String.get text 0 |> int_of_char in
          if ch >= 0x20 then Some (Key_down UInt8.(of_int ch)) else None
        else None
      end
      else None
    else None
  end
