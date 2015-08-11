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

let interact_with_devices computer =
  let open Lwt in
  let open Computer in

  let interact_with_instance c (module I : Device.Instance) =
    I.Device.on_interaction I.this |> map (fun updated_this ->
        let manifest =
          Manifest.update
            (module struct
              include I

              let this = updated_this
            end : Device.Instance)
            c.manifest
        in
        { c with manifest })
  in
  let instances = Manifest.all computer.manifest in
  Lwt_monad.fold interact_with_instance computer instances

let handle_event computer =
  Input_event.poll () >>= function
  | Some Input_event.Quit -> exit 0
  | _ -> interact_with_devices computer

let make_monitor_window () =
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

        make_monitor_window () >>= fun window ->

        Clock.make () >>= fun clock ->

        let monitor = Monitor.make window in

        let manifest = Manifest.(
            empty
            |> register (module Clock) clock
            |> register (module Monitor) monitor)
        in

        let computer = { Computer.default with memory; manifest } in

        let runtime () =
          Engine.launch ~computer ~suspend_every:frame_period
            handle_event
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
