open Prelude

open Unsigned

type t =
  | Binary of Code.t * Address.t * Address.t
  | Unary of Special_code.t * Address.t

let rec execute ins =
  let open Computer_state.Monad in

  match ins with
  | Binary (code, address_b, address_a) -> begin
      Computer_state.of_program (execute_binary code address_b address_a) >>= fun () ->
      Computer_state.unit (Code.conditional code)
    end
  | Unary (code, address_a) -> begin
      execute_unary code address_a >>= fun () ->
      Computer_state.unit false
    end

and execute_binary code address_b address_a =
  let open Code in
  let open Program in
  let open Program.Functor in
  let open Program.Monad in

  let apply f =
    Address.get address_a >>= fun y ->
    Address.get address_b >>= fun x ->
    Address.set (f x y) address_b
  in

  let apply_to_signed f =
    Address.get address_a |> map Word.to_int >>= fun ys ->
    Address.get address_b |> map Word.to_int >>= fun xs ->
    let zs = f xs ys in
    Address.set (Word.of_int zs) address_b
  in

  let apply_to_double f action =
    Address.get address_a |> map (fun a -> UInt16.of_int (Word.to_int a)) >>= fun yd ->
    Address.get address_b |> map (fun b -> UInt16.of_int (Word.to_int b)) >>= fun xd ->
    let zd = f xd yd in
    Address.set (word (UInt16.to_int zd)) address_b >>= fun () ->
    action zd >>= fun () ->
    Return ()
  in

  let skip_unless test =
    Address.get address_a >>= fun y ->
    Address.get address_b >>= fun x ->
    set_flag Cpu.Flag.Skip_next (not (test x y))
  in

  match code with
  | Set -> Address.get address_a >>= fun a -> Address.set a address_b
  | Add -> apply_to_double (fun x y -> UInt16.Infix.(x + y))
             (fun z ->
                write_special
                  Special.EX
                  (if UInt16.(z > of_int 0xffff) then word 1 else word 0))
  | Sub -> apply_to_double (fun x y -> UInt16.Infix.(x - y))
             (fun z ->
                write_special
                  Special.EX
                  (if UInt16.(z > of_int 0xffff) then word 0xffff else word 0))
  | Mul -> apply_to_double (fun x y -> UInt16.Infix.(x * y))
             (fun z ->
                write_special
                  Special.EX
                  Word.(of_int UInt16.(to_int UInt16.Infix.((z lsr 16) land of_int 0xffff))))
  | Mli -> apply_to_signed (fun x y -> x * y)
  | Div -> begin
      Address.get address_a |> map (fun a -> UInt16.of_int (Word.to_int a)) >>= fun yd ->
      Address.get address_b |> map (fun b -> UInt16.of_int (Word.to_int b)) >>= fun xd ->

      if yd = UInt16.zero then
        Address.set (word 0) address_b >>= fun () ->
        write_special Special.EX (word 0)
      else
        Address.set (Word.of_int UInt16.Infix.((xd / yd) |> UInt16.to_int)) address_b >>= fun () ->
        write_special
          Special.EX
          (Word.of_int UInt16.Infix.(((xd lsl 16) / yd) land UInt16.of_int 0xffff |> UInt16.to_int))
    end
  | Dvi -> apply_to_signed (fun x y -> x / y)
  | Bor -> apply (fun x y -> Word.(x lor y))
  | Shl -> begin
      Address.get address_a |> map (fun a -> UInt16.of_int (Word.to_int a)) >>= fun yd ->
      Address.get address_b |> map (fun b -> UInt16.of_int (Word.to_int b)) >>= fun xd ->
      let zd = UInt16.Infix.(xd lsl (UInt16.to_int yd)) |> UInt16.to_int |> word in
      Address.set zd address_b
    end
  | Ife -> skip_unless (fun x y -> x = y)
  | Ifn -> skip_unless (fun x y -> x <> y)

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
