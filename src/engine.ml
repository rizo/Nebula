(** Launch the DCPU-16. *)

open Prelude

exception Bad_decoding of word * Computer.t

exception No_such_device of word * Computer.t

exception Stack_overflow of Computer.t

exception Stack_underflow of Computer.t

(** Jump to the interrupt handler and disable interrupt dequeing. *)
let handle_interrupt (Interrupt.Message message) c =
  let open Computer in
  let open Program.Monad in

  Computer_state.of_program begin
    let open Program in
    read_special Special.PC >>= push >>= fun () ->
    read_register Reg.A >>= push >>= fun () ->
    read_special Special.IA >>= write_special Special.PC >>= fun () ->
    write_register Reg.A message
  end
  |> Computer_state.run
    { c with
      interrupt_ctrl = Interrupt_control.disable_dequeuing c.interrupt_ctrl }
  |> fst

(** Execute a triggered interrupt.

    Software interrupts are enqueued directly. Hardware interrupts cause the
    interrupt hook for the device registered at the interrupt index to be
    invoked. *)
let execute_interrupt trigger c =
  let open Computer in
  let open IO.Functor in
  let open IO.Monad in

  match trigger with
  | Interrupt.Trigger.Software message -> begin
      let interrupt_ctrl =
        Interrupt_control.enqueue (Interrupt.Message message) c.interrupt_ctrl
      in
      IO.unit { c with interrupt_ctrl }
    end
  | Interrupt.Trigger.Hardware index -> begin
      IO.lift (fun () -> Manifest.instance index c.manifest) >>= fun (module I : Device.Instance) ->
      I.Device.on_interrupt I.this
      |> IO.Functor.map (fun program ->
          program
          |> Computer_state.of_program
          |> Computer_state.run c
          |> fun (c, device) ->
          { c with
            manifest =
              Manifest.update
                (Device.make_instance (module I.Device) device I.index)
                c.manifest
          })
    end

(** Execute an iteration of the DCPU-16.

    First, dequeue an interrupt (if one is waiting) and transfer control to the interrupt handler.

    Next, decode the next instruction from memory at the program counter.

    Finally, check for triggered interrupts (either by software, or by a device
    when it was "ticked") and execute them. *)
let step c =
  let open Computer in
  let open IO.Monad in

  IO.lift begin fun () ->
    let c =
      if Cpu.read_special Special.IA c.cpu != word 0 then
        match Interrupt_control.handle c.interrupt_ctrl with
        | None -> c
        | Some (interrupt, interrupt_ctrl) -> handle_interrupt interrupt { c with interrupt_ctrl }
      else c
    in

    let skipping = Cpu.get_flag Cpu.Flag.Skip_next c.cpu in

    let (c_after_execution, continue_skipping) =
      let open Computer_state.Monad in
      let open Computer_state.Functor in

      try
        let s = Computer_state.of_program Program.next_word >>= fun w ->
          match Decode.instruction w with
          | None -> raise (Bad_decoding (w, c))
          | Some ins -> begin
              Instruction.execute ins
              |> map (fun conditional_instruction -> skipping && conditional_instruction)
            end
        in
        Computer_state.run c s
      with
      | Program.Stack_underflow -> raise (Stack_underflow c)
      | Program.Stack_overflow -> raise (Stack_overflow c)
    in

    print_endline (Computer.show c);

    (* If we're skipping instructions, then we only care about updating the
       position of the program counter. That we can do this in this way is the
       beauty of immutable data structures. *)
    if skipping then begin
      print_endline "Skipped!";
      { c with
        cpu =
          c.cpu
          |> Cpu.write_special Special.PC (Cpu.read_special Special.PC c_after_execution.cpu)
          |> Cpu.set_flag Cpu.Flag.Skip_next continue_skipping
      }
    end
    else c_after_execution

  end >>= fun c ->

  match Interrupt_control.triggered c.interrupt_ctrl with
  | None -> IO.unit c
  | Some (trigger, interrupt_ctrl) -> begin
      IO.catch
        (execute_interrupt trigger { c with interrupt_ctrl })
        (function
          | Manifest.No_such_device index -> IO.throw (No_such_device (index, c))
          | error -> IO.throw error)
    end

(** "Tick" all devices in the manifest. *)
let tick_devices c =
  let open Computer in
  let open IO.Functor in

  let tick_instance c (module I : Device.Instance) =
    I.Device.on_tick I.this |> map (fun (updated_this, generated_interrupt) ->
        let manifest =
          Manifest.update
            (Device.make_instance (module I.Device) updated_this I.index)
            c.manifest
        in
        let interrupt_ctrl =
          match generated_interrupt with
          | Some interrupt -> Interrupt_control.enqueue interrupt c.interrupt_ctrl
          | None -> c.interrupt_ctrl
        in
        { c with manifest; interrupt_ctrl })
  in
  let instances = Manifest.all c.manifest in
  IO.Monad.fold tick_instance c instances

(**  Launch a DCPU-16.

     Control is suspended every [suspend_every] nanoseconds to [suspension]. Otherwise, this
     computation will never terminate unless it throws. *)
let launch ~computer ~suspend_every ~suspension =
  let open IO.Monad in
  let open IO.Functor in

  let rec loop last_suspension_time computer =
    tick_devices computer >>= fun computer ->

    step computer >>= fun computer ->

    Precision_clock.get_time >>= fun now ->
    let elapsed = now - last_suspension_time in

    let next =
      if elapsed >= suspend_every then
        suspension computer |> map (fun computer -> now, computer)
      else
        IO.unit (last_suspension_time, computer)
    in
    next >>= fun (time, computer) -> loop time computer
  in
  Precision_clock.get_time >>= fun now -> loop now computer
