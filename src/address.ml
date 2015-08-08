open Prelude

type t =
  | Reg_direct of Reg.t
  | Reg_indirect of Reg.t
  | Reg_indirect_offset of Reg.t
  | Push
  | Pop
  | Peek
  | Pick
  | SP
  | PC
  | EX
  | Direct
  | Indirect
  | Literal of word

open Program
open Program.Monad
open Special

exception Invalid_operation

let get = function
  | Reg_direct r -> read_register r
  | Reg_indirect r -> read_register r >>= read_memory
  | Reg_indirect_offset r -> begin
      next_word >>= fun n ->
      read_register r >>= fun v ->
      read_memory Word.(n + v)
    end
  | Push -> raise Invalid_operation
  | Pop -> pop
  | Peek -> read_special SP >>= read_memory
  | Pick -> begin
      next_word >>= fun n ->
      read_special SP >>= fun sp ->
      read_memory Word.(n + sp)
    end
  | SP -> read_special SP
  | PC -> read_special PC
  | EX -> read_special EX
  | Direct -> next_word
  | Indirect -> next_word >>= read_memory
  | Literal v -> Program.Return v

let set value = function
  | Reg_direct r -> write_register r value
  | Reg_indirect r -> read_register r >>= fun offset -> write_memory offset value
  | Reg_indirect_offset r -> begin
      next_word >>= fun n ->
      read_register r >>= fun v ->
      write_memory Word.(n + v) value
    end
  | Push -> push value
  | Pop -> raise Invalid_operation
  | Peek -> read_special SP >>= fun sp -> write_memory sp value
  | Pick -> begin
      next_word >>= fun n ->
      read_special SP >>= fun sp ->
      write_memory Word.(sp + n) value
    end
  | SP -> write_special SP value
  | PC -> write_special PC value
  | EX -> write_special EX value
  | Direct -> Return ()
  | Indirect -> next_word >>= fun n -> write_memory n value
  | Literal v -> Return ()
