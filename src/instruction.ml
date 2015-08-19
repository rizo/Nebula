open Prelude

type t =
  | Binary of Code.t * Address.t * Address.t
  | Unary of Special_code.t * Address.t

let rec execute = function
  | Binary (code, address_b, address_a) -> Computer_state.of_program
                                             (execute_binary code address_b address_a)
  | Unary (code, address_a) -> execute_unary code address_a

and execute_binary code address_b address_a =
  let open Code in
  let open Program in
  let open Program.Monad in
  match code with
  | Set -> Address.get address_a >>= fun a -> Address.set a address_b
  | Add -> begin
      Address.get address_a >>= fun a ->
      Address.get address_b >>= fun b ->
      Address.set Word.(a + b) address_b
    end

and execute_unary code address_a =
  let open Computer in
  let open Computer_state in
  let open Computer_state.Monad in
  let open Program in
  let open Special_code in

  let state_a = of_program (Address.get address_a) in

  match code with
  | Jsr -> begin
      state_a >>= fun a ->
      of_program begin
        let open Program.Monad in
        read_special Special.PC >>= push >>= fun () ->
        write_special Special.PC a
      end
    end
  | Int -> begin
      let open Interrupt_control in
      state_a >>= fun a ->
      modify begin function
          { interrupt_ctrl; _ } as c ->
          { c with
            interrupt_ctrl =
              trigger (Interrupt.Trigger.Software a) interrupt_ctrl
          }
      end
    end
  | Ias -> state_a >>= Computer_state.write_special Special.IA
  | Iag -> Computer_state.read_special Special.IA >>= Computer_state.write_register Reg.A
  | Rfi -> begin
      let open Interrupt_control in
      of_program begin
        let open Program.Monad in
        pop >>= write_register Reg.A >>= fun () ->
        pop >>= write_special Special.PC
      end >>= fun () ->
      modify begin function
        | { interrupt_ctrl; _ } as c ->
          { c with interrupt_ctrl = Interrupt_control.enable_dequeuing interrupt_ctrl }
      end
    end
  | Iaq -> begin
      state_a >>= fun a ->
      modify begin function
        | { interrupt_ctrl; _ } as c ->
          { c with
            interrupt_ctrl =
              (if a <> word 0 then
                Interrupt_control.disable_dequeuing
              else Interrupt_control.enable_dequeuing) interrupt_ctrl }
      end
    end
  | Hwn -> modify begin fun c ->
      { c with
        cpu = Manifest.size c.manifest |> Word.of_int |> fun w -> Cpu.write_register Reg.A w c.cpu
      }
    end
  | Hwq -> begin
      state_a >>= fun index ->
      get >>= fun c ->
      let info = Manifest.query index c.manifest in
      Computer_state.write_register Reg.A Device.Info.(snd info.id) >>= fun () ->
      Computer_state.write_register Reg.B Device.Info.(fst info.id) >>= fun () ->
      Computer_state.write_register Reg.C Device.Info.(info.version) >>= fun () ->
      Computer_state.write_register Reg.X Device.Info.(snd info.manufacturer) >>= fun () ->
      Computer_state.write_register Reg.Y Device.Info.(fst info.manufacturer)
    end
  | Hwi -> begin
      let open Interrupt_control in
      state_a >>= fun a ->
      modify begin function
          { interrupt_ctrl; _ } as c ->
          { c with
            interrupt_ctrl =
              trigger (Interrupt.Trigger.Hardware a) interrupt_ctrl
          }
      end
    end
  | Abt -> begin
      print_endline "Aborting.";
      exit 0
    end
  | Dbg -> begin
      state_a >>= fun a ->
      let time = Unix.gettimeofday () in
      print_endline (Printf.sprintf "[%f] %s" time (Word.show a));
      flush stdout;
      unit ()
    end
