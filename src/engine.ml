 open Prelude

exception Bad_decoding of word * Computer.t

exception Stack_overflow of Computer.t

exception Stack_underflow of Computer.t

let handle_interrupt (Interrupt.Message message) t =
  let open Computer in
  let open Program.Monad in

  let ia = t.cpu |> Cpu.read_special Special.IA in
  if ia = word 0 then t
  else
    Computer_state.of_program begin
      let open Program in
      read_special Special.PC >>= push >>= fun () ->
      read_register Reg.A >>= push >>= fun () ->
      write_special Special.PC ia >>= fun () ->
      write_register Reg.A message
    end |> Computer_state.run t |> fst

let step t =
  let open Computer in
  let open Computer_state.Monad in

  let t =
    match Interrupt_control.triggered t.interrupt_ctrl with
    | None -> t
    | Some (trigger, interrupt_ctrl) ->
      match trigger with
      | Interrupt.Trigger.Software message ->
        let interrupt_ctrl =
          Interrupt_control.receive (Interrupt.Message message) interrupt_ctrl
        in
        { t with interrupt_ctrl }

      | Interrupt.Trigger.Hardware index ->
        print_endline ("Sending HW interrupt to " ^ (Word.show index));
        t
  in

  let t =
    match Interrupt_control.handle t.interrupt_ctrl with
    | None -> t
    | Some (interrupt, interrupt_ctrl) ->
      let t = handle_interrupt interrupt t in
      { t with interrupt_ctrl }
  in

  try
    let s = Computer_state.of_program Program.next_word >>= fun w ->
      match Decode.instruction w with
      | None -> raise (Bad_decoding (w, t))
      | Some ins -> Instruction.execute ins
    in
    Computer_state.run t s |> fst
  with
  | Program.Stack_underflow -> raise (Stack_underflow t)
  | Program.Stack_overflow -> raise (Stack_overflow t)

let rec launch t =
  launch (step t)
