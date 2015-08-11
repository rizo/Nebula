open Prelude

open Lwt

let initialize () =
  Sdl.lift "Initializing: %s" (fun () -> Sdl.init Sdl.Init.(video + events))

let show_error_and_exit ?computer message =
  Lwt_io.printf "ERROR: %s\n" message >>= fun () ->

  (match computer with
  | Some c -> Lwt_io.print ("\n" ^ Computer.show c)
  | None -> return_unit) >>= fun () ->

  return (exit 1)

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

let render window =
  Lwt_monad.sequence_unit Visual.[
    set_color window Color.white;
    clear window;
    set_color window Color.red;
    rectangle window ~origin:(0, 0) ~width:50 ~height:50;
    render window
    ]

let handle_event window =
  Input_event.poll () >>= function
  | Some Input_event.Quit -> exit 0
  | _ -> render window

let prepare_manifest () =
  Clock.make () >>= fun clock ->

  Lwt.return Manifest.(
    empty
    |> register (module Clock) clock)

let monitor_window () =
  Visual.Window.make ~width:640 ~height:480 ~title:"DCPU-16 Monitor"

let frame_period =
  16666667

let main file_name =
  Lwt_main.run begin
    let open Computer in

    Mem.of_file file_name >>= function
    | Left (`Bad_memory_file message) -> show_error_and_exit ("Reading memory file " ^ message)
    | Right memory -> begin
        initialize () >>= fun () ->
        monitor_window () >>= fun window ->
        prepare_manifest () >>= fun manifest ->

        let computer = { Computer.default with memory; manifest } in

        let runtime () =
          Engine.launch ~computer ~suspend_every:frame_period (fun computer ->
              handle_event window >>= fun () -> return computer)
        in

        Lwt.catch
          runtime
          (fun error -> handle_error error)
      end
  end

let () =
  match Cli.parse_to main with
  | `Error _ -> exit 1
  | _ -> exit 0
