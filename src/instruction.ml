open Prelude

open Unsigned

type t =
  | Binary of Code.t * Address.t * Address.t
  | Unary of Special_code.t * Address.t

let conditional = function
  | Unary _ -> false
  | Binary (code, _, _) -> Code.conditional code

let encoded_size = function
  | Binary (code, b, a) -> 1 + Address.extra_encoded_size b + Address.extra_encoded_size a
  | Unary (code, a) -> 1 + Address.extra_encoded_size a

let rec execute ins =
  let open Computer_state.Monad in

  Computer_state.get_flag Cpu.Flag.Skip_next >>= fun skipping ->
  if skipping then begin
    let open Computer_state in
    read_special Special.PC >>= fun pc ->
    write_special Special.PC Word.(pc + word (encoded_size ins) - word 1) >>= fun () ->
    set_flag Cpu.Flag.Skip_next (conditional ins)
  end else
    match ins with
    | Binary (code, address_b, address_a) -> execute_binary code address_b address_a
    | Unary (code, address_a) -> execute_unary code address_a

and execute_binary code address_b address_a =
  let open Code in
  let open Computer_state in
  let open Computer_state.Functor in
  let open Computer_state.Monad in

  Address.target_of address_a >>= fun ta ->
  Address.target_of address_b >>= fun tb ->

  let apply f =
    Address.Target.get ta >>= fun y ->
    Address.Target.get tb >>= fun x ->
    Address.Target.set (f x y) tb
  in

  let apply_to_signed f =
    Address.Target.get ta |> map Word.to_int >>= fun ys ->
    Address.Target.get tb |> map Word.to_int >>= fun xs ->
    let zs = f xs ys in
    Address.Target.set (Word.of_int zs) tb
  in

  let apply_to_double f action =
    Address.Target.get ta |> map Word.to_dword >>= fun yd ->
    Address.Target.get tb |> map Word.to_dword >>= fun xd ->
    let zd = f xd yd in
    Address.Target.set (Word.of_dword zd) tb >>= fun () ->
    action zd
  in

  let skip_unless test =
    Address.Target.get ta >>= fun y ->
    Address.Target.get tb >>= fun x ->
    set_flag Cpu.Flag.Skip_next (not (test x y))
  in

  let signed_skip_unless test =
    Address.Target.get ta |> map Word.to_int >>= fun yi ->
    Address.Target.get tb |> map Word.to_int >>= fun xi ->
    set_flag Cpu.Flag.Skip_next (not (test xi yi))
  in

  match code with
  | Set -> Address.Target.get ta >>= fun y -> Address.Target.set y tb
  | Add -> apply_to_double (fun x y -> UInt32.Infix.(x + y))
             (fun z ->
                write_special
                  Special.EX
                  (if UInt32.(z > of_int 0xffff) then word 1 else word 0))
  | Sub -> apply_to_double (fun x y -> UInt32.Infix.(x - y))
             (fun z ->
                write_special
                  Special.EX
                  (if UInt32.(z > of_int 0xffff) then word 0xffff else word 0))
  | Mul -> apply_to_double (fun x y -> UInt32.Infix.(x * y))
             (fun z ->
                write_special
                  Special.EX
                  Word.(of_int UInt32.(to_int UInt32.Infix.((z lsr 16) land of_int 0xffff))))
  | Mli -> apply_to_signed (fun x y -> x * y)
  | Div -> begin
      Address.Target.get ta |> map Word.to_dword >>= fun yd ->
      Address.Target.get tb |> map Word.to_dword >>= fun xd ->

      if yd = UInt32.zero then
        Address.Target.set (word 0) tb >>= fun () ->
        write_special Special.EX (word 0)
      else
        Address.Target.set (Word.of_dword UInt32.Infix.(xd / yd)) tb >>= fun () ->
        write_special
          Special.EX
          (Word.of_dword UInt32.Infix.(((xd lsl 16) / yd) land UInt32.of_int 0xffff))
    end
  | Dvi -> apply_to_signed (fun xi yi -> xi / yi)
  | Mod -> apply begin fun x y ->
      if y = word 0 then word 0
      else
        Word.(x mod y)
    end
  | Mdi -> apply_to_signed (fun xi yi -> if yi = 0 then 0 else xi mod yi)
  | And -> apply (fun x y -> Word.(x land y))
  | Bor -> apply (fun x y -> Word.(x lor y))
  | Xor -> apply (fun x y -> Word.(x lxor y))
  | Shr -> begin
      Address.Target.get ta |> map Word.to_int >>= fun yi ->
      Address.Target.get tb |> map Word.to_dword >>= fun xd ->
      let zd = UInt32.Infix.(xd lsr yi) |> Word.of_dword in
      Address.Target.set zd tb >>= fun () ->
      write_special
        Special.EX
        (UInt32.Infix.((xd lsl 16) lsr yi) |> Word.of_dword)
    end
  | Asr -> begin
      Address.Target.get ta |> map Word.to_int >>= fun yi ->
      Address.Target.get tb |> map (fun w -> w |> Word.to_int |> UInt32.of_int) >>= fun xdi ->
      let zdi = UInt32.Infix.(xdi lsr yi) in
      Address.Target.set (Word.of_dword zdi) tb >>= fun () ->
      write_special
        Special.EX
        (UInt32.Infix.((xdi lsl 16) lsr yi) |> Word.of_dword)
    end
  | Shl -> begin
      Address.Target.get ta |> map Word.to_int >>= fun yi ->
      Address.Target.get tb |> map Word.to_dword >>= fun xd ->
      let zd = UInt32.Infix.(xd lsl yi) |> Word.of_dword in
      Address.Target.set zd tb
    end
  | Ifb -> skip_unless (fun x y -> Word.(x land y) <> word 0)
  | Ifc -> skip_unless (fun x y -> Word.(x land y) = word 0)
  | Ife -> skip_unless (fun x y -> x = y)
  | Ifn -> skip_unless (fun x y -> x <> y)
  | Ifg -> skip_unless (fun x y -> Word.(x > y))
  | Ifa -> signed_skip_unless (fun x y -> x > y)
  | Ifl -> skip_unless (fun x y -> Word.(x < y))
  | Ifu -> skip_unless (fun x y -> x < y)
  | Adx -> begin
      Address.Target.get ta |> map Word.to_dword >>= fun yd ->
      Address.Target.get tb |> map Word.to_dword >>= fun xd ->
      read_special Special.EX |> map Word.to_dword >>= fun exd ->
      let zd = UInt32.Infix.(xd + yd + exd) in

      Address.Target.set (Word.of_dword zd) tb >>= fun () ->
      write_special
        Special.EX
        (if UInt32.(zd > of_int 0xffff) then word 1 else word 0)
    end
  | Sbx -> begin
      Address.Target.get ta |> map Word.to_dword >>= fun yd ->
      Address.Target.get tb |> map Word.to_dword >>= fun xd ->
      read_special Special.EX |> map Word.to_dword >>= fun exd ->
      let zd = UInt32.Infix.(xd - yd + exd) in

      Address.Target.set (Word.of_dword zd) tb >>= fun () ->
      write_special
        Special.EX
        (if UInt32.(zd > of_int 0xffff) then word 1 else word 0)
    end
  | Sti -> begin
      Address.Target.get ta >>= fun y ->
      Address.Target.set y tb >>= fun () ->
      read_register Reg.I >>= fun i -> write_register Reg.I Word.(i + word 1) >>= fun () ->
      read_register Reg.J >>= fun j -> write_register Reg.J Word.(j + word 1)
    end
  | Std -> begin
      Address.Target.get ta >>= fun y ->
      read_register Reg.I >>= fun i ->
      read_register Reg.J >>= fun j ->
      Address.Target.set y tb >>= fun () ->
      write_register Reg.I Word.(i + word 1) >>= fun () ->
      write_register Reg.I Word.(j + word 1)
    end

and execute_unary code address_a =
  let open Computer in
  let open Computer_state in
  let open Computer_state.Monad in
  let open Program in
  let open Special_code in

  Address.target_of address_a >>= fun ta ->
  Address.Target.get ta >>= fun a ->

  match code with
  | Jsr -> begin
      of_program begin
        let open Program.Monad in
        read_special Special.PC >>= push >>= fun () ->
        write_special Special.PC a
      end
    end
  | Int -> begin
      let open Interrupt_control in
      modify begin function
          { interrupt_ctrl; _ } as c ->
          { c with
            interrupt_ctrl =
              trigger (Interrupt.Trigger.Software a) interrupt_ctrl
          }
      end
    end
  | Ias -> Computer_state.write_special Special.IA a
  | Iag -> Computer_state.read_special Special.IA >>= fun w -> Address.Target.set w ta
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
      modify begin function
        | { interrupt_ctrl; _ } as c ->
          { c with
            interrupt_ctrl =
              (if a <> word 0 then
                Interrupt_control.disable_dequeuing
              else Interrupt_control.enable_dequeuing) interrupt_ctrl }
      end
    end
  | Hwn -> begin
      get >>= fun c ->
      Manifest.size c.manifest |> Word.of_int |> fun w ->
      Address.Target.set w ta
    end
  | Hwq -> begin
      let index = a in
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
      let time = Unix.gettimeofday () in
      print_endline (Printf.sprintf "[%f] %s" time (Word.show a));
      flush stdout;
      unit ()
    end
