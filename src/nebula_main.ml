open IO.Functor
open IO.Monad
open Prelude

open Unsigned

open Printf

let initialize =
  Sdl.lift "Initializing: %s" (fun () -> Sdl.init Sdl.Init.(video + events))

let show_error_and_exit ?computer message =
  IO.put_string (sprintf "ERROR: %s\n" message) >>= fun () ->

  (match computer with
  | Some c -> IO.put_string ("\n" ^ Computer.show c)
  | None -> IO.unit ()) >>= fun () ->

  IO.terminate 1

let handle_error error =
  let open Format in

  match error with
  | Engine.Bad_decoding (w, computer) ->
    begin
      show_error_and_exit
        (sprintf "Failed to decode word %s." (Word.show w)) ~computer
    end
  | Engine.Stack_overflow computer -> show_error_and_exit "Stack overflow." ~computer
  | Engine.Stack_underflow computer -> show_error_and_exit "Stack underflow" ~computer
  | Engine.No_such_device (index, computer) ->
    begin
      show_error_and_exit
        (sprintf "Index %s is not associated with a device." (Word.show index))
        ~computer
    end
  | _ -> show_error_and_exit
           (sprintf "Unexpected failure: %s\n" (Printexc.to_string error))

let interact_with_devices device_input computer =
  let open Computer in

  let interact_with_instance c (module I : Device.Instance) =
    I.Device.on_interaction device_input c.memory I.this |> map (fun updated_this ->
        let manifest =
          Manifest.update
            (Device.make_instance (module I.Device) updated_this I.index)
            c.manifest
        in
        { c with manifest })
  in
  let instances = Manifest.all computer.manifest in
  IO.Monad.fold interact_with_instance computer instances

let handle_event computer =
  Input_event.poll >>= function
  | Some Input_event.Quit -> IO.terminate 0
  | Some Input_event.Key_down code -> interact_with_devices
                                        Device.Input.{ key_code = Some code }
                                        computer
  | _ -> interact_with_devices Device.Input.none computer

let make_monitor_window =
  Visual.Window.make
    ~width:Monitor.total_width
    ~height:Monitor.total_height
    ~title:"DCPU-16 Monitor"

let frame_period =
  30000000

let main file_name =
  IO.main begin
    let open Computer in

    Mem.of_file file_name >>= function
    | Left (`Bad_memory_file message) -> show_error_and_exit ("Reading memory file " ^ message)
    | Right memory -> begin
        initialize >>= fun () ->
        make_monitor_window >>= fun window ->

        let keyboard = Keyboard.make in
        Clock.make >>= fun clock ->
        Monitor.make window >>= fun monitor ->

        let manifest = Manifest.(
            empty
            |> register (module Clock) clock
            |> register (module Keyboard) keyboard
            |> register (module Monitor) monitor)
        in

        let computer = { Computer.default with memory; manifest } in

        IO.catch
          (Engine.launch ~computer ~suspend_every:frame_period ~suspension:handle_event)
          handle_error
      end
  end

let () =
  match Cli.parse_to main with
  | `Error _ -> exit 1
  | _ -> exit 0
