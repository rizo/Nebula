(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

open Functional

module C = Computer
module Cs = Computer_state
module Ic = Interrupt_control
module P = Program

exception Bad_decoding of word * Computer_state.t

exception No_such_device of word * Computer_state.t

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

let execute_trigger trigger c =
  let open IO.Functor in
  let open IO.Monad in

  match trigger with
  | Interrupt.Trigger.Software message -> begin
      let ic = Ic.enqueue (Interrupt.Message message) c.Cs.ic in
      IO.unit Cs.{ c with ic }
    end
  | Interrupt.Trigger.Hardware index -> begin
      IO.lift (fun () -> Manifest.get_record index c.Cs.manifest) >>= fun r ->

      r.Manifest.Record.device#on_interrupt
      |> IO.Functor.map begin fun program ->
          program
          |> C.of_program
          |> C.run c
          |> fun (c, device) ->
          Cs.{ c with
               manifest = Manifest.update Manifest.Record.{ r with device } c.manifest }
      end
    end

let step c =
  let unsafe_step c =
    let open IO.Monad in

    let c =
      if Cpu.read_special Special.IA c.Cs.cpu != word 0 then
        match Ic.handle c.Cs.ic with
        | None -> c
        | Some (interrupt, ic) -> jump_to_handler interrupt Cs.{ c with ic }
      else c
    in

    IO.lift begin fun () ->
      let open C.Monad in
      let open C.Functor in

      let s = C.of_program P.next_word >>= fun w ->
        match Decode.instruction w with
        | None -> raise (Bad_decoding (w, c))
        | Some ins -> Instruction.execute ins
      in
      C.run c s |> fst
    end >>= fun c ->
    match Ic.triggered c.Cs.ic with
    | None -> IO.unit c
    | Some (trigger, ic) -> execute_trigger trigger Cs.{ c with ic }
  in

  IO.catch (unsafe_step c)
    (function
      | Manifest.No_such_device index -> IO.throw (No_such_device (index, c))
      | error -> IO.throw error)

let tick_devices c =
  let open IO.Functor in

  let tick_device c r =
    r.Manifest.Record.device#on_tick |> map (fun (device, generated_interrupt) ->
        let manifest = Manifest.update Manifest.Record.{ r with device } c.Cs.manifest in
        let ic =
          match generated_interrupt with
          | Some interrupt -> Ic.enqueue interrupt c.Cs.ic
          | None -> c.Cs.ic
        in
        Cs.{ c with manifest; ic })
  in
  let instances = Manifest.all c.Cs.manifest in
  IO.Monad.fold tick_device c instances

let interact_with_devices device_input c =
  let open IO.Functor in

  let interact_with_device c r =
    r.Manifest.Record.device#on_interaction device_input c.Cs.memory
    |> map begin fun device ->
        let manifest = Manifest.update Manifest.Record.{ r with device } c.Cs.manifest in
        Cs.{ c with manifest }
    end
  in
  let records = Manifest.all c.Cs.manifest in
  IO.Monad.fold interact_with_device c records

let launch ~suspend_every ~suspension c =
  let open IO.Monad in
  let open IO.Functor in

  let rec loop last_suspension_time c =
    tick_devices c >>= step >>= fun c ->

    Precision_clock.get_time >>= fun now ->
    let elapsed = Duration.of_nanoseconds Time_stamp.(now - last_suspension_time) in

    let next =
      if elapsed >= suspend_every then
        suspension c |> map (fun c -> now, c)
      else
        IO.unit (last_suspension_time, c)
    in
    next >>= fun (time, c) -> loop time c
  in
  Precision_clock.get_time >>= fun now -> loop now c
