(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

open Functional

module C = Computer
module Cs = Computer_state
module Ic = Interrupt_control
module P = Program

exception Bad_decoding of word * Computer.t

exception No_such_device of word * Computer.t

let jump_to_handler (Interrupt.Message message) c =
  let open P.Monad in

  Cs.of_program begin
    P.read_special Special.PC >>= P.push >>= fun () ->
    P.read_register Reg.A >>= P.push >>= fun () ->
    P.read_special Special.IA >>= P.write_special Special.PC >>= fun () ->
    P.write_register Reg.A message
  end
  |> Cs.run { c with C.ic = Ic.disable_dequeuing c.C.ic }
  |> fst

let execute_trigger trigger c =
  let open IO.Functor in
  let open IO.Monad in

  match trigger with
  | Interrupt.Trigger.Software message -> begin
      let ic = Ic.enqueue (Interrupt.Message message) c.C.ic in
      IO.unit C.{ c with ic }
    end
  | Interrupt.Trigger.Hardware index -> begin
      IO.lift begin fun () ->
        Manifest.instance index c.C.manifest
      end >>= fun (module I : Device.Instance) ->
      I.Device.on_interrupt I.this
      |> IO.Functor.map (fun program ->
          program
          |> Cs.of_program
          |> Cs.run c
          |> fun (c, device) ->
          C.{ c with
            manifest =
              Manifest.update
                (Device.make_instance (module I.Device) device I.index)
                c.manifest
          })
    end

let step c =
  let unsafe_step c =
    let open IO.Monad in

    let c =
      if Cpu.read_special Special.IA c.C.cpu != word 0 then
        match Ic.handle c.C.ic with
        | None -> c
        | Some (interrupt, ic) -> jump_to_handler
                                    interrupt
                                    C.{ c with ic }
      else c
    in

    IO.lift begin fun () ->
      let open Cs.Monad in
      let open Cs.Functor in

      let s = Cs.of_program P.next_word >>= fun w ->
        match Decode.instruction w with
        | None -> raise (Bad_decoding (w, c))
        | Some ins -> Instruction.execute ins
      in
      Cs.run c s |> fst
    end >>= fun c ->
    match Ic.triggered c.C.ic with
    | None -> IO.unit c
    | Some (trigger, ic) -> execute_trigger trigger C.{ c with ic }
  in

  IO.catch (unsafe_step c)
    (function
      | Manifest.No_such_device index -> IO.throw (No_such_device (index, c))
      | error -> IO.throw error)

let tick_devices c =
  let open IO.Functor in

  let tick_instance c (module I : Device.Instance) =
    I.Device.on_tick I.this |> map (fun (updated_this, generated_interrupt) ->
        let manifest =
          Manifest.update
            (Device.make_instance (module I.Device) updated_this I.index)
            c.C.manifest
        in
        let ic =
          match generated_interrupt with
          | Some interrupt -> Ic.enqueue interrupt c.C.ic
          | None -> c.C.ic
        in
        C.{ c with manifest; ic })
  in
  let instances = Manifest.all c.C.manifest in
  IO.Monad.fold tick_instance c instances

let launch ~suspend_every ~suspension c =
  let open IO.Monad in
  let open IO.Functor in

  let rec loop last_suspension_time c =
    tick_devices c >>= step >>= fun c ->

    Precision_clock.get_time >>= fun now ->
    let elapsed = Duration.of_nanoseconds (Int64.sub now last_suspension_time) in

    let next =
      if elapsed >= suspend_every then
        suspension c |> map (fun c -> now, c)
      else
        IO.unit (last_suspension_time, c)
    in
    next >>= fun (time, c) ->
    loop time c
  in
  Precision_clock.get_time >>= fun now -> loop now c
