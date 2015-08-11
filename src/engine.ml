open Prelude

exception Bad_decoding of word * Computer.t

exception No_such_device of word * Computer.t

exception Stack_overflow of Computer.t

exception Stack_underflow of Computer.t

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

let trigger_interrupt trigger c =
  let open Computer in

  match trigger with
  | Interrupt.Trigger.Software message -> begin
      let interrupt_ctrl =
        Interrupt_control.enqueue (Interrupt.Message message) c.interrupt_ctrl
      in
      { c with interrupt_ctrl }
    end
  | Interrupt.Trigger.Hardware index -> begin
      let (module I : Device.Instance) = Manifest.instance index c.manifest in
      let (c, device) =
        Computer_state.of_program (I.Device.on_interrupt index I.this)
        |> Computer_state.run c
      in
      { c with
        manifest =
          Manifest.update
            (module struct
              include I

              let this = device
            end : Device.Instance)
            c.manifest
      }
    end

let step c =
  let open Computer in
  let open Computer_state.Monad in

  let c =
    if Cpu.read_special Special.IA c.cpu != word 0 then
      match Interrupt_control.handle c.interrupt_ctrl with
      | None -> c
      | Some (interrupt, interrupt_ctrl) -> handle_interrupt interrupt { c with interrupt_ctrl }
    else c
  in

  let c =
    try
      let s = Computer_state.of_program Program.next_word >>= fun w ->
        match Decode.instruction w with
        | None -> raise (Bad_decoding (w, c))
        | Some ins -> Instruction.execute ins
      in
      Computer_state.run c s |> fst
    with
    | Program.Stack_underflow -> raise (Stack_underflow c)
    | Program.Stack_overflow -> raise (Stack_overflow c)
  in

  let c =
    match Interrupt_control.triggered c.interrupt_ctrl with
    | None -> c
    | Some (trigger, interrupt_ctrl) -> begin
        try
          trigger_interrupt trigger { c with interrupt_ctrl }
        with
        | Manifest.No_such_device index -> raise (No_such_device (index, c))
      end
  in
  c

let tick_devices c =
  let open Lwt in
  let open Computer in

  let visit_instance c (module I : Device.Instance) =
    I.Device.on_tick I.this |> map (fun (updated_this, generated_interrupt) ->
        let manifest =
          Manifest.update
            (module struct
              include I

              let this = updated_this
            end : Device.Instance)
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
  Lwt_monad.fold visit_instance c instances

let launch ~computer ~suspend_every suspend =
  let open Lwt in

  let rec loop last_suspension_time computer =
    tick_devices computer >>= fun computer ->
    wrap (fun () -> step computer) >>= fun computer ->

    Precision_clock.get_time () >>= fun now ->
    let elapsed = now - last_suspension_time in

    let next () =
      if elapsed >= suspend_every then
        suspend computer |> map (fun computer -> now, computer)
      else
        return (last_suspension_time, computer)
    in
    next () >>= fun (time, computer) -> loop time computer
  in

  Precision_clock.get_time () >>= fun now -> loop now computer
