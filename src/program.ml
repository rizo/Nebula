(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Functional
open Prelude

module Op = struct
  type 'a t =
    | Read_memory of word * (word -> 'a)
    | Write_memory of word * word * 'a
    | Read_register of Reg.t * (word -> 'a)
    | Write_register of Reg.t * word * 'a
    | Read_special of Special.t * (word -> 'a)
    | Write_special of Special.t * word * 'a
    | Set_flag of Cpu.Flag.t * bool * 'a
    | Get_flag of Cpu.Flag.t * (bool -> 'a)

  let map f = function
    | Read_memory (o, next) -> Read_memory (o, fun w -> f (next w))
    | Write_memory (o, v, next) -> Write_memory (o, v, f next)
    | Read_register (r, next) -> Read_register (r, fun v -> f (next v))
    | Write_register (r, v, next) -> Write_register (r, v, f next)
    | Read_special (s, next) -> Read_special (s, fun v -> f (next v))
    | Write_special (s, v, next) -> Write_special (s, v, f next)
    | Set_flag (flag, v, next) -> Set_flag (flag, v, f next)
    | Get_flag (flag, next) -> Get_flag (flag, fun v -> f (next v))
end

include Free.Make(Op)

let read_memory offset =
  lift (Op.Read_memory (offset, id))

let write_memory offset value =
  lift (Op.Write_memory (offset, value, ()))

let read_register reg =
  lift (Op.Read_register (reg, id))

let write_register reg value =
  lift (Op.Write_register (reg, value, ()))

let read_special s =
  lift (Op.Read_special (s, id))

let write_special s value =
  lift (Op.Write_special (s, value, ()))

let get_flag flag =
  lift (Op.Get_flag (flag, id))

let set_flag flag value =
  lift (Op.Set_flag (flag, value, ()))

let next_word =
  let open Monad in
  read_special Special.PC >>= fun pc ->
  write_special Special.PC Word.(pc + word 1) >>= fun () ->
  read_memory pc

let push value =
  let open Monad in
  let open Special in
  read_special SP >>= fun sp ->
  let sp = Word.(sp - word 1) in
  write_special SP sp >>= fun () ->
  write_memory sp value

let pop =
  let open Monad in
  let open Special in
  read_special SP >>= fun sp ->
  write_special SP Word.(sp + word 1) >>= fun () ->
  read_memory sp

let read_extent offset n =
  let low = Word.to_int offset in
  let offsets = enum_from_to low (low + n) in
  List.map (fun i -> read_memory (word i)) offsets |> Monad.sequence
