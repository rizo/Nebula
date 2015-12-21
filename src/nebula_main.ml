(** Main entry point for Nebula.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

open Functional
open Functional.Prelude

open Unsigned

open Printf

module Cs = Computer_state

open Functional.IO.Functor
open Functional.IO.Monad

(** Must be called at the beginning of Nebula's execution. *)
let initialize =
  Sdl.lift "Initializing: %s" (fun () -> Sdl.init Sdl.Init.(video + events))

let show_failure_and_exit ?computer message =
  IO.put_string (sprintf "Error: %s\n" message) >>= fun () ->

  (match computer with
  | Some c -> IO.put_string ("\n" ^ Cs.show c)
  | None -> IO.unit ()) >>= fun () ->

  IO.terminate 1

(** Print a helpful error message and then terminate. *)
let handle_error error =
  match error with
  | Engine.Bad_decoding (w, computer) -> begin
      show_failure_and_exit
        (sprintf "Failed to decode word %s." (Word.show w)) ~computer
    end
  | Engine.No_such_device (index, computer) -> begin
      show_failure_and_exit
        (sprintf "Index %s is not associated with a device." (Word.show index))
        ~computer
    end
  | _ -> show_failure_and_exit
           (sprintf "Unexpected failure: %s\n" (Printexc.to_string error))

let handle_event c =
  Input_event.poll >>= function
  | Some Input_event.Quit -> IO.terminate 0
  | Some Input_event.Key_down code -> begin
      Engine.interact_with_devices Device.Input.{ key_code = Some code } c
    end
  | _ -> Engine.interact_with_devices Device.Input.none c

let make_monitor_window =
  Visual.Window.make
    ~width:Devices.Monitor.total_width
    ~height:Devices.Monitor.total_height
    ~title:"DCPU-16 Monitor"

(** The period between rendering graphics and processing input events. *)
let frame_period =
  Duration.of_nanoseconds 30000000L

let main file_name =
  IO.main begin
    Mem.of_file file_name >>= function
    | Left (`Bad_memory_file message) -> begin
        show_failure_and_exit ("Reading memory file " ^ message)
      end
    | Right memory -> begin
        initialize >>= fun () ->
        make_monitor_window >>= fun window ->

        let keyboard = Devices.Keyboard.make in
        Devices.Clock.make >>= fun clock ->
        Devices.Monitor.make window >>= fun monitor ->

        let manifest = Manifest.(
            empty
            |> register clock
            |> register keyboard
            |> register monitor)
        in

        let c = Cs.{ default with memory; manifest } in

        IO.catch
          (Engine.launch ~suspend_every:frame_period ~suspension:handle_event c)
          handle_error
      end
  end

let () =
  match Cli.parse_to main with
  | `Error _ -> exit 1
  | _ -> exit 0
