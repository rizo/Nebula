(** Main entry point for Nebula.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Functional
open Prelude

open Unsigned

open Printf

open IO.Functor
open IO.Monad

(** Must be called at the beginning of Nebula's execution. *)
let initialize =
  Sdl.lift "Initializing: %s" (fun () -> Sdl.init Sdl.Init.(video + events))

let show_error_and_exit ?computer message =
  IO.put_string (sprintf "ERROR: %s\n" message) >>= fun () ->

  (match computer with
  | Some c -> IO.put_string ("\n" ^ Computer.show c)
  | None -> IO.unit ()) >>= fun () ->

  IO.terminate 1

(** Print a helpful error message and then terminate. *)
let handle_error error =
  match error with
  | Engine.Bad_decoding (w, computer) -> begin
      show_error_and_exit
        (sprintf "Failed to decode word %s." (Word.show w)) ~computer
    end
  | Engine.No_such_device (index, computer) -> begin
      show_error_and_exit
        (sprintf "Index %s is not associated with a device." (Word.show index))
        ~computer
    end
  | _ -> show_error_and_exit
           (sprintf "Unexpected failure: %s\n" (Printexc.to_string error))

let interact_with_devices device_input c =
  let interact_with_instance c (module I : Device.Instance) =
    I.Device.on_interaction device_input c.Computer.memory I.this |> map (fun updated_this ->
        let manifest =
          Manifest.update
            (Device.make_instance (module I.Device) updated_this I.index)
            c.Computer.manifest
        in
        Computer.{ c with manifest })
  in
  let instances = Manifest.all c.Computer.manifest in
  IO.Monad.fold interact_with_instance c instances

let handle_event c =
  Input_event.poll >>= function
  | Some Input_event.Quit -> IO.terminate 0
  | Some Input_event.Key_down code -> interact_with_devices
                                        Device.Input.{ key_code = Some code }
                                        c
  | _ -> interact_with_devices Device.Input.none c

let make_monitor_window =
  Visual.Window.make
    ~width:Devices.Monitor.total_width
    ~height:Devices.Monitor.total_height
    ~title:"DCPU-16 Monitor"

(** The period between rendering graphics and processing input events. Expressed
    in nanoseconds. *)
let frame_period =
  30000000

let main file_name =
  IO.main begin
    Mem.of_file file_name >>= function
    | Left (`Bad_memory_file message) -> show_error_and_exit ("Reading memory file " ^ message)
    | Right memory -> begin
        initialize >>= fun () ->
        make_monitor_window >>= fun window ->

        let keyboard = Devices.Keyboard.make in
        Devices.Clock.make >>= fun clock ->
        Devices.Monitor.make window >>= fun monitor ->

        let manifest = Manifest.(
            empty
            |> register (module Devices.Clock) clock
            |> register (module Devices.Keyboard) keyboard
            |> register (module Devices.Monitor) monitor)
        in

        let c = Computer.{ default with memory; manifest } in

        IO.catch
          (Engine.launch ~suspend_every:frame_period ~suspension:handle_event c)
          handle_error
      end
  end

let () =
  match Cli.parse_to main with
  | `Error _ -> exit 1
  | _ -> exit 0
