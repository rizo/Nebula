(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

open Unsigned

module C = Computer
module Cs = Computer_state
module Ic = Interrupt_control
module P = Program

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
  let open C.Monad in

  C.of_program (P.get_flag Cpu.Flag.Skip_next) >>= fun skipping ->
  if skipping then C.of_program begin
      let open P.Monad in

      P.read_special Special.PC >>= fun pc ->
      P.write_special Special.PC Word.(pc + word (encoded_size ins) - word 1) >>= fun () ->
      P.set_flag Cpu.Flag.Skip_next (conditional ins)
    end
  else
    match ins with
    | Binary (code, address_b, address_a) -> execute_binary code address_b address_a
    | Unary (code, address_a) -> execute_unary code address_a

and execute_binary code address_b address_a =
  let open C.Functor in
  let open C.Monad in

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
    C.of_program (action zd)
  in

  let skip_unless test =
    Address.Target.get ta >>= fun y ->
    Address.Target.get tb >>= fun x ->
    C.of_program (P.set_flag Cpu.Flag.Skip_next (not (test x y)))
  in

  let signed_skip_unless test =
    Address.Target.get ta |> map Word.to_int >>= fun yi ->
    Address.Target.get tb |> map Word.to_int >>= fun xi ->
    C.of_program (P.set_flag Cpu.Flag.Skip_next (not (test xi yi)))
  in

  match code with
  | Code.Set -> Address.Target.get ta >>= fun y -> Address.Target.set y tb
  | Code.Add -> apply_to_double (fun x y -> UInt32.Infix.(x + y))
                  (fun z ->
                     P.write_special
                       Special.EX
                       (if UInt32.(z > of_int 0xffff) then word 1 else word 0))
  | Code.Sub -> apply_to_double (fun x y -> UInt32.Infix.(x - y))
                  (fun z ->
                     P.write_special
                       Special.EX
                       (if UInt32.(z > of_int 0xffff) then word 0xffff else word 0))
  | Code.Mul -> apply_to_double (fun x y -> UInt32.Infix.(x * y))
                  (fun z ->
                     P.write_special
                       Special.EX
                       Word.(of_int UInt32.(to_int UInt32.Infix.((z lsr 16) land of_int 0xffff))))
  | Code.Mli -> apply_to_signed (fun x y -> x * y)
  | Code.Div -> begin
      Address.Target.get ta |> map Word.to_dword >>= fun yd ->
      Address.Target.get tb |> map Word.to_dword >>= fun xd ->

      if yd = UInt32.zero then
        Address.Target.set (word 0) tb >>= fun () ->
        C.of_program (P.write_special Special.EX (word 0))
      else
        Address.Target.set (Word.of_dword UInt32.Infix.(xd / yd)) tb >>= fun () ->
        C.of_program begin
          P.write_special
            Special.EX
            (Word.of_dword UInt32.Infix.(((xd lsl 16) / yd) land UInt32.of_int 0xffff))
        end
    end
  | Code.Dvi -> apply_to_signed (fun xi yi -> xi / yi)
  | Code.Mod -> apply begin fun x y ->
      if y = word 0 then word 0
      else
        Word.(x mod y)
    end
  | Code.Mdi -> apply_to_signed (fun xi yi -> if yi = 0 then 0 else xi mod yi)
  | Code.And -> apply (fun x y -> Word.(x land y))
  | Code.Bor -> apply (fun x y -> Word.(x lor y))
  | Code.Xor -> apply (fun x y -> Word.(x lxor y))
  | Code.Shr -> begin
      Address.Target.get ta |> map Word.to_int >>= fun yi ->
      Address.Target.get tb |> map Word.to_dword >>= fun xd ->
      let zd = UInt32.Infix.(xd lsr yi) |> Word.of_dword in
      Address.Target.set zd tb >>= fun () ->
      C.of_program begin
        P.write_special
          Special.EX
          (UInt32.Infix.((xd lsl 16) lsr yi) |> Word.of_dword)
      end
    end
  | Code.Asr -> begin
      Address.Target.get ta |> map Word.to_int >>= fun yi ->
      Address.Target.get tb |> map (fun w -> w |> Word.to_int |> UInt32.of_int) >>= fun xdi ->
      let zdi = UInt32.Infix.(xdi lsr yi) in
      Address.Target.set (Word.of_dword zdi) tb >>= fun () ->
      C.of_program begin
        P.write_special
          Special.EX
          (UInt32.Infix.((xdi lsl 16) lsr yi) |> Word.of_dword)
      end
    end
  | Code.Shl -> begin
      Address.Target.get ta |> map Word.to_int >>= fun yi ->
      Address.Target.get tb |> map Word.to_dword >>= fun xd ->
      let zd = UInt32.Infix.(xd lsl yi) |> Word.of_dword in
      Address.Target.set zd tb
    end
  | Code.Ifb -> skip_unless (fun x y -> Word.(x land y) <> word 0)
  | Code.Ifc -> skip_unless (fun x y -> Word.(x land y) = word 0)
  | Code.Ife -> skip_unless (fun x y -> x = y)
  | Code.Ifn -> skip_unless (fun x y -> x <> y)
  | Code.Ifg -> skip_unless (fun x y -> Word.(x > y))
  | Code.Ifa -> signed_skip_unless (fun x y -> x > y)
  | Code.Ifl -> skip_unless (fun x y -> Word.(x < y))
  | Code.Ifu -> skip_unless (fun x y -> x < y)
  | Code.Adx -> begin
      Address.Target.get ta |> map Word.to_dword >>= fun yd ->
      Address.Target.get tb |> map Word.to_dword >>= fun xd ->
      C.of_program (P.read_special Special.EX) |> map Word.to_dword >>= fun exd ->
      let zd = UInt32.Infix.(xd + yd + exd) in

      Address.Target.set (Word.of_dword zd) tb >>= fun () ->
      C.of_program begin
        P.write_special
          Special.EX
          (if UInt32.(zd > of_int 0xffff) then word 1 else word 0)
      end
    end
  | Code.Sbx -> begin
      Address.Target.get ta |> map Word.to_dword >>= fun yd ->
      Address.Target.get tb |> map Word.to_dword >>= fun xd ->
      C.of_program (P.read_special Special.EX) |> map Word.to_dword >>= fun exd ->
      let zd = UInt32.Infix.(xd - yd + exd) in

      Address.Target.set (Word.of_dword zd) tb >>= fun () ->
      C.of_program begin
        P.write_special
          Special.EX
          (if UInt32.(zd > of_int 0xffff) then word 1 else word 0)
      end
    end
  | Code.Sti -> begin
      Address.Target.get ta >>= fun y ->
      Address.Target.set y tb >>= fun () ->
      C.of_program begin
        let open P.Monad in
        P.read_register Reg.I >>= fun i ->
        P.write_register Reg.I Word.(i + word 1) >>= fun () ->
        P.read_register Reg.J >>= fun j ->
        P.write_register Reg.J Word.(j + word 1)
      end
    end
  | Code.Std -> begin
      Address.Target.get ta >>= fun y ->
      Address.Target.set y tb >>= fun () ->
      C.of_program begin
        let open P.Monad in
        P.read_register Reg.I >>= fun i ->
        P.read_register Reg.J >>= fun j ->
        P.write_register Reg.I Word.(i + word 1) >>= fun () ->
        P.write_register Reg.I Word.(j + word 1)
      end
    end

and execute_unary code address_a =
  let open C.Monad in

  Address.target_of address_a >>= fun ta ->
  Address.Target.get ta >>= fun a ->

  match code with
  | Special_code.Jsr -> begin
      C.of_program begin
        let open P.Monad in
        P.read_special Special.PC >>= P.push >>= fun () ->
        P.write_special Special.PC a
      end
    end
  | Special_code.Int -> begin
      let open Interrupt_control in
      C.modify begin function
          { Cs.ic; _ } as c ->
          Cs.{ c with
               ic =
                 trigger (Interrupt.Trigger.Software a) ic
             }
      end
    end
  | Special_code.Ias -> C.of_program (P.write_special Special.IA a)
  | Special_code.Iag -> begin
      C.of_program (P.read_special Special.IA) >>= fun w ->
      Address.Target.set w ta
    end
  | Special_code.Rfi -> begin
      C.of_program begin
        let open P.Monad in
        P.pop >>= P.write_register Reg.A >>= fun () ->
        P.pop >>= P.write_special Special.PC
      end >>= fun () ->
      C.modify begin function
        | { Cs.ic; _ } as c ->
          Cs.{ c with ic = Ic.enable_dequeuing ic }
      end
    end
  | Special_code.Iaq -> begin
      C.modify begin function
        | { Cs.ic; _ } as c ->
          Cs.{ c with
               ic =
                 (if a <> word 0 then
                    Ic.disable_dequeuing
                  else Ic.enable_dequeuing) ic
             }
      end
    end
  | Special_code.Hwn -> begin
      C.get >>= fun c ->
      Manifest.size c.Cs.manifest |> Word.of_int |> fun w ->
      Address.Target.set w ta
    end
  | Special_code.Hwq -> begin
      let index = a in
      C.get >>= fun c ->
      let info = Manifest.query index c.Cs.manifest in
      C.of_program begin
        let open P.Monad in
        P.write_register Reg.A Device.Info.(snd info.id) >>= fun () ->
        P.write_register Reg.B Device.Info.(fst info.id) >>= fun () ->
        P.write_register Reg.C Device.Info.(info.version) >>= fun () ->
        P.write_register Reg.X Device.Info.(snd info.manufacturer) >>= fun () ->
        P.write_register Reg.Y Device.Info.(fst info.manufacturer)
      end
    end
  | Special_code.Hwi -> begin
      C.modify begin function
          { Cs.ic; _ } as c ->
          Cs.{ c with
            ic =
              Ic.trigger (Interrupt.Trigger.Hardware a) ic
          }
      end
    end
  | Special_code.Abt -> begin
      C.of_program begin
        let open P.Functor in
        let open P.Monad in

        P.read_memory a |> map Word.to_int >>= fun error_length ->
        P.read_extent Word.(a + word 1) error_length >>= fun message_words ->

        print_string "Run-time error: ";
        message_words |> List.iter (fun w -> print_char (Char.chr (Word.to_int w)));
        print_newline ();
        print_endline "Aborting.";
        exit 0 |> ignore;

        P.Return ()
      end
    end
  | Special_code.Dbg -> begin
      let time = Unix.gettimeofday () in
      print_endline (Printf.sprintf "[%f] %s" time (Word.show a));
      flush stdout;
      C.unit ()
    end
