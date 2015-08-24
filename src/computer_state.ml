(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Functional
open Prelude

include State.Make(Computer)

open Computer

let read_register reg =
  gets (fun t -> Cpu.read_register reg t.cpu)

let write_register reg value =
  modify (fun t -> { t with cpu = Cpu.write_register reg value t.cpu })

let read_special s =
  gets (fun t -> Cpu.read_special s t.cpu)

let write_special s value =
  modify (fun t -> { t with cpu = Cpu.write_special s value t.cpu })

let get_flag flag =
  gets (fun t -> Cpu.get_flag flag t.cpu)

let set_flag flag value =
  modify (fun t -> { t with cpu = Cpu.set_flag flag value t.cpu })

let read_memory offset =
  gets (fun t -> Mem.read offset t.memory)

let write_memory offset value =
  modify (fun t -> { t with memory = Mem.write offset value t.memory })

let rec of_program t =
  let open Monad in

  match t with
  | Program.Suspend (Program.Op.Read_memory (o, next)) ->
    read_memory o >>= fun value -> of_program (next value)

  | Program.Suspend (Program.Op.Write_memory (o, v, next)) ->
    write_memory o v >>= fun () -> of_program next

  | Program.Suspend (Program.Op.Read_register (r, next)) ->
    read_register r >>= fun value -> of_program (next value)

  | Program.Suspend (Program.Op.Write_register (r, v, next)) ->
    write_register r v >>= fun () -> of_program next

  | Program.Suspend (Program.Op.Read_special (s, next)) ->
    read_special s >>= fun value -> of_program (next value)

  | Program.Suspend (Program.Op.Write_special (s, v, next)) ->
    write_special s v >>= fun () -> of_program next

  | Program.Suspend (Program.Op.Set_flag (flag, v, next)) ->
    set_flag flag v >>= fun () -> of_program next

  | Program.Suspend (Program.Op.Get_flag (flag, next)) ->
    get_flag flag >>= fun value -> of_program (next value)

  | Program.Return v -> unit v
