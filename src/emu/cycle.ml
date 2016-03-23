(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

open Functional

module C = Computer
module Cs = Computer_state
module Ic = Interrupt_control
module P = Program

(** A {! word} could not be decoded by the DCPU-16. *)
exception Bad_decoding of word * Computer_state.t

(** A device was requested at an index that has not been assigned. *)
exception No_such_device of word * Computer_state.t

let update_manifest r c =
  Cs.{ c with
       manifest = Manifest.(update r c.manifest)
     }

let send_interrupt_to_device r c =
  r.Manifest.Record.device#on_interrupt
  |> IO.Functor.map begin fun program ->
    program
    |> C.of_program
    |> C.run c
    |> fun (c, device) ->
    update_manifest Manifest.Record.{ r with device } c
  end

let enqueue_interrupt_message m c =
  let ic = Ic.enqueue (Interrupt.Message m) c.Cs.ic in
  Cs.{ c with ic }

let execute_trigger trigger c =
  match trigger with
  | Interrupt.Trigger.Software m -> IO.unit (enqueue_interrupt_message m c)
  | Interrupt.Trigger.Hardware index -> begin
      let r = Manifest.get_record index c.Cs.manifest in
      send_interrupt_to_device r c
    end

let execute_trigger_if_present c =
  match Ic.triggered c.Cs.ic with
  | None -> IO.unit c
  | Some (trigger, ic) -> execute_trigger trigger Cs.{ c with ic }

let jump_to_handler (Interrupt.Message message) c =
  let open P.Monad in

  C.of_program begin
    P.read_special Special.PC >>= P.push >>= fun () ->
    P.read_register Reg.A >>= P.push >>= fun () ->
    P.read_special Special.IA >>= P.write_special Special.PC >>= fun () ->
    P.write_register Reg.A message
  end
  |> C.run { c with Cs.ic = Ic.disable_dequeuing c.Cs.ic }
  |> fst

let interrupt_handler_set c =
  Cpu.read_special Special.IA c.Cs.cpu != word 0

let handle_interrupt c =
  if interrupt_handler_set c then
    match Ic.dequeue c.Cs.ic with
    | None -> c
    | Some (interrupt, ic) -> jump_to_handler interrupt Cs.{ c with ic }
  else c

let decode_and_execute c =
  let open C.Monad in

  let s = C.of_program P.next_word >>= fun w ->
    match Decode.instruction w with
    | None -> raise (Bad_decoding (w, c))
    | Some ins -> Instruction.execute ins
  in
  C.run c s |> fst

let tick_device c r =
  r.Manifest.Record.device#on_tick
  |> IO.Functor.map (fun (device, generated_interrupt) ->
      let manifest = Manifest.(update Record.{ r with device } c.Cs.manifest) in
      let ic =
        match generated_interrupt with
        | Some interrupt -> Ic.enqueue interrupt c.Cs.ic
        | None -> c.Cs.ic
      in
      Cs.{ c with manifest; ic })

let tick_devices c =
  let records = Manifest.all c.Cs.manifest in
  IO.Monad.fold tick_device c records

let unsafe_step c =
  let open IO.Monad in

  let c = handle_interrupt c in
  let c = decode_and_execute c in
  execute_trigger_if_present c >>=
  tick_devices

let invoke c =
  IO.catch (unsafe_step c)
    (function
      | Manifest.No_such_device index -> IO.throw (No_such_device (index, c))
      | error -> IO.throw error)
