open Prelude

exception Bad_decoding of word * Computer.t

exception No_such_device of word * Computer.t

exception Stack_overflow of Computer.t

exception Stack_underflow of Computer.t

let handle_interrupt (Interrupt.Message message) c =
  let open Computer in
  let open Program.Monad in

  let ia = c.cpu |> Cpu.read_special Special.IA in
  if ia = word 0 then c
  else
    Computer_state.of_program begin
      let open Program in
      read_special Special.PC >>= push >>= fun () ->
      read_register Reg.A >>= push >>= fun () ->
      write_special Special.PC ia >>= fun () ->
      write_register Reg.A message
    end |> Computer_state.run c |> fst

let trigger_interrupt trigger c =
  let open Computer in

  match trigger with
  | Interrupt.Trigger.Software message ->
    begin
      let interrupt_ctrl =
        Interrupt_control.receive (Interrupt.Message message) c.interrupt_ctrl
      in
      { c with interrupt_ctrl }
    end
  | Interrupt.Trigger.Hardware index ->
    begin
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
    match Interrupt_control.triggered c.interrupt_ctrl with
    | None -> c
    | Some (trigger, interrupt_ctrl) ->
      begin
        try
          let c = trigger_interrupt trigger c in
          { c with interrupt_ctrl }
        with
        | Manifest.No_such_device index -> raise (No_such_device (index, c))
      end
  in

  let c =
    match Interrupt_control.handle c.interrupt_ctrl with
    | None -> c
    | Some (interrupt, interrupt_ctrl) ->
      let c = handle_interrupt interrupt c in
      { c with interrupt_ctrl }
  in

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

let visit_devices c =
  let open Computer in
  let open IO.Monad in

  let visit_instance c (module I : Device.Instance) =
    I.Device.on_visit I.this |> IO.Functor.map (fun (this, generated_interrupt) ->
        let manifest =
          Manifest.update
            (module struct
              include I

              let this = this
            end : Device.Instance)
            c.manifest
        in
        let interrupt_ctrl =
          match generated_interrupt with
          | Some interrupt -> Interrupt_control.receive interrupt c.interrupt_ctrl
          | None -> c.interrupt_ctrl
        in
        { c with manifest; interrupt_ctrl })
  in
  let instances = Manifest.all c.manifest in
  IO.Monad.fold visit_instance c instances

let rec launch c =
  let open IO.Monad in

  visit_devices c >>= fun c ->
  try
    let next = step c in
    launch next
  with
  | error -> IO.unit error
