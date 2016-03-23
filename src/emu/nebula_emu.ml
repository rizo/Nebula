(** Main entry point for Nebula.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

open Functional
open Functional.Prelude

open Unsigned

module Cs = Computer_state

open Functional.IO.Functor
open Functional.IO.Monad

let sprintf =
  Printf.sprintf

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
  | Cycle.Bad_decoding (w, computer) -> begin
      show_failure_and_exit ~computer
        (sprintf "Failed to decode word %s." (Word.show w))
    end
  | Cycle.No_such_device (index, computer) -> begin
      show_failure_and_exit ~computer
        (sprintf "Index %s is not associated with a device." (Word.show index))
    end
  | Engine.Invalid_operation (_, computer) -> begin
      show_failure_and_exit ~computer
        "The computer is in an invalid state."
    end
  | _ -> show_failure_and_exit
           (sprintf "Unexpected failure: %s\n" (Printexc.to_string error))

let make_monitor_window =
  Visual.Window.make
    ~width:Device_monitor.total_width
    ~height:Device_monitor.total_height
    ~title:"DCPU-16 Monitor"
  >>= fun window ->
  Visual.set_color window Visual.Color.black >>= fun () ->
  IO.unit window

(** The period between rendering graphics and processing input events. *)
let frame_period =
  Duration.of_nanoseconds 30000000L

module Interaction = struct
  let interact_with_device device_input c r =
    r.Manifest.Record.device#on_interaction device_input c.Cs.memory
    |> IO.Functor.map begin fun device ->
      let manifest = Manifest.(update Record.{ r with device } c.Cs.manifest) in
      Cs.{ c with manifest }
    end

  let interact_with_devices device_input c =
    let records = Manifest.all c.Cs.manifest in
    IO.Monad.fold (interact_with_device device_input) c records

  let invoke c =
    Input_event.poll >>= function
    | Some Input_event.Quit -> IO.terminate 0
    | Some Input_event.Key_down code -> begin
        interact_with_devices Device.Input.{ key_code = Some code } c
      end
    | _ -> interact_with_devices Device.Input.none c
end

module Make (Clock : Clock.S) (Engine : Engine.S) = struct
  let init ~file_name =
    IO.main begin
      Mem.of_file file_name >>= function
      | Left (`Bad_memory_file message) -> begin
          show_failure_and_exit ("Reading memory file " ^ message)
        end
      | Right memory -> begin
          initialize >>= fun () ->
          make_monitor_window >>= fun window ->

          let keyboard = Device_keyboard.make in
          Device_clock.make (module Clock) >>= fun clock ->
          Device_monitor.make (module Clock) window >>= fun monitor ->

          let manifest = Manifest.(
              empty
              |> register clock
              |> register keyboard
              |> register monitor)
          in

          let c = Cs.{ default with memory; manifest } in

          IO.catch
            (Engine.launch ~suspend_every:frame_period c)
            handle_error
        end
    end
end

let () =
  match Cli.parse_to begin
      fun file_name start_interactive ->
        let module Clock = Clock.Precision in

        let (module Control) =
          Engine.control_with_initial_state
            (module Control)
            (if start_interactive then
               Control.empty_with_unconditional_hit
             else Control.empty)
        in

        let module Engine =
          Engine.Make
            (Clock)
            (Control)
            (Cycle)
            (Interaction)
        in

        let module M = Make (Clock) (Engine) in
        M.init ~file_name
    end
  with
  | `Error _ -> exit 1
  | _ -> exit 0
