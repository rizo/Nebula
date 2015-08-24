(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Functional
open Prelude

module C = Computer_state
module P = Program

exception Bad_decoding of word * Computer.t

exception No_such_device of word * Computer.t

let handle_interrupt (Interrupt.Message message) c =
  let open Program.Monad in

  C.of_program begin
    P.read_special Special.PC >>= P.push >>= fun () ->
    P.read_register Reg.A >>= P.push >>= fun () ->
    P.read_special Special.IA >>= P.write_special Special.PC >>= fun () ->
    P.write_register Reg.A message
  end
  |> Computer_state.run
    { c with
      Computer.interrupt_ctrl =
        Interrupt_control.disable_dequeuing c.Computer.interrupt_ctrl }
  |> fst

let execute_interrupt trigger c =
  let open IO.Functor in
  let open IO.Monad in

  match trigger with
  | Interrupt.Trigger.Software message -> begin
      let interrupt_ctrl =
        Interrupt_control.enqueue (Interrupt.Message message) c.Computer.interrupt_ctrl
      in
      IO.unit Computer.{ c with interrupt_ctrl }
    end
  | Interrupt.Trigger.Hardware index -> begin
      IO.lift begin fun () ->
        Manifest.instance index c.Computer.manifest
      end >>= fun (module I : Device.Instance) ->
      I.Device.on_interrupt I.this
      |> IO.Functor.map (fun program ->
          program
          |> Computer_state.of_program
          |> Computer_state.run c
          |> fun (c, device) ->
          Computer.{ c with
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
      if Cpu.read_special Special.IA c.Computer.cpu != word 0 then
        match Interrupt_control.handle c.Computer.interrupt_ctrl with
        | None -> c
        | Some (interrupt, interrupt_ctrl) -> handle_interrupt
                                                interrupt
                                                Computer.{ c with interrupt_ctrl }
      else c
    in

    IO.lift begin fun () ->
      let open Computer_state.Monad in
      let open Computer_state.Functor in

      let s = Computer_state.of_program Program.next_word >>= fun w ->
        match Decode.instruction w with
        | None -> raise (Bad_decoding (w, c))
        | Some ins -> Instruction.execute ins
      in
      Computer_state.run c s |> fst
    end >>= fun c ->
    match Interrupt_control.triggered c.Computer.interrupt_ctrl with
    | None -> IO.unit c
    | Some (trigger, interrupt_ctrl) -> execute_interrupt
                                          trigger
                                          Computer.{ c with interrupt_ctrl }
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
            c.Computer.manifest
        in
        let interrupt_ctrl =
          match generated_interrupt with
          | Some interrupt -> Interrupt_control.enqueue interrupt c.Computer.interrupt_ctrl
          | None -> c.Computer.interrupt_ctrl
        in
        Computer.{ c with manifest; interrupt_ctrl })
  in
  let instances = Manifest.all c.Computer.manifest in
  IO.Monad.fold tick_instance c instances

let launch ~suspend_every ~suspension c =
  let open IO.Monad in
  let open IO.Functor in

  let rec loop last_suspension_time c =
    tick_devices c >>= step >>= fun c ->

    Precision_clock.get_time >>= fun now ->
    let elapsed = now - last_suspension_time in

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
