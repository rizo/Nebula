(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

open Functional

include State.Make(Computer)

module C = Computer

let rec of_program t =
  let open Monad in

  match t with
  | Program.Suspend (Program.Op.Read_register (r, next)) -> begin
      gets (fun t -> Cpu.read_register r t.C.cpu) >>= fun value ->
      of_program (next value)
    end
  | Program.Suspend (Program.Op.Write_register (r, v, next)) -> begin
      modify (fun t -> C.{ t with cpu = Cpu.write_register r v t.cpu }) >>= fun () ->
      of_program next
    end
  | Program.Suspend (Program.Op.Read_special (s, next)) -> begin
      gets (fun t -> Cpu.read_special s t.C.cpu) >>= fun value ->
      of_program (next value)
    end
  | Program.Suspend (Program.Op.Write_special (s, v, next)) -> begin
      modify (fun t -> C.{ t with cpu = Cpu.write_special s v t.cpu }) >>= fun () ->
      of_program next
    end
  | Program.Suspend (Program.Op.Read_memory (o, next)) -> begin
      gets (fun t -> Mem.read o t.C.memory) >>= fun value ->
      of_program (next value)
    end
  | Program.Suspend (Program.Op.Write_memory (o, v, next)) -> begin
      modify (fun t -> C.{ t with memory = Mem.write o v t.memory }) >>= fun () ->
      of_program next
    end
  | Program.Suspend (Program.Op.Get_flag (flag, next)) -> begin
      gets (fun t -> Cpu.get_flag flag t.C.cpu) >>= fun value ->
      of_program (next value)
    end
  | Program.Suspend (Program.Op.Set_flag (flag, v, next)) -> begin
      modify (fun t -> C.{ t with cpu = Cpu.set_flag flag v t.cpu }) >>= fun () ->
      of_program next
    end
  | Program.Return v -> unit v
