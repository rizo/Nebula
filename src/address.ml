(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Prelude

open Program
open Program.Monad
open Special

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

exception Invalid_operation

let extra_encoded_size = function
  | Reg_direct _ -> 0
  | Reg_indirect _ -> 0
  | Reg_indirect_offset _ -> 1
  | Push -> 0
  | Pop -> 0
  | Peek -> 0
  | Pick -> 1
  | SP -> 0
  | PC -> 0
  | EX -> 0
  | Direct -> 1
  | Indirect -> 1
  | Literal _ -> 0

module Target = struct
  open Computer_state

  type t =
    | Reg of Reg.t
    | Special of Special.t
    | Offset of word
    | Push
    | Pop
    | Value of word

  let get = function
    | Reg r -> read_register r
    | Special s -> read_special s
    | Offset o -> read_memory o
    | Push -> raise Invalid_operation
    | Pop -> of_program pop
    | Value v -> unit v

  let set v = function
    | Reg r -> write_register r v
    | Special s -> write_special s v
    | Offset o -> write_memory o v
    | Push -> of_program (push v)
    | Pop -> raise Invalid_operation
    | Value _ -> unit ()
end

let target_of t =
  Computer_state.of_program begin
    match t with
    | Reg_direct r -> Return (Target.Reg r)
    | Reg_indirect r -> read_register r >>= fun w -> Return (Target.Offset w)
    | Reg_indirect_offset r -> begin
        next_word >>= fun n ->
        read_register r >>= fun w ->
        Return (Target.Offset Word.(n + w))
      end
    | Push -> Return (Target.Push)
    | Pop -> Return (Target.Pop)
    | Peek -> read_special PC >>= fun w -> Return (Target.Offset w)
    | Pick -> begin
        next_word >>= fun n ->
        read_special SP >>= fun sp ->
        Return (Target.Offset Word.(n + sp))
      end
    | SP -> Return (Target.Special SP)
    | PC -> Return (Target.Special PC)
    | EX -> Return (Target.Special EX)
    | Direct -> next_word >>= fun n -> Return (Target.Value n)
    | Indirect -> next_word >>= fun n -> Return (Target.Offset n)
    | Literal v -> Return (Target.Value v)
  end
