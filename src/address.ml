(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

module P = Program
module C = Computer

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

  let get t =
    Computer_state.of_program begin
      match t with
        | Reg r -> P.read_register r
        | Special s -> P.read_special s
        | Offset o -> P.read_memory o
        | Push -> raise Invalid_operation
        | Pop -> P.pop
        | Value v -> P.Return v
    end

  let set v t =
    Computer_state.of_program begin
      match t with
      | Reg r -> P.write_register r v
      | Special s -> P.write_special s v
      | Offset o -> P.write_memory o v
      | Push -> P.push v
      | Pop -> raise Invalid_operation
      | Value _ -> P.Return ()
    end
end

let target_of t =
  let open Program.Monad in

  Computer_state.of_program begin
    match t with
    | Reg_direct r -> P.Return (Target.Reg r)
    | Reg_indirect r -> P.read_register r >>= fun w -> P.Return (Target.Offset w)
    | Reg_indirect_offset r -> begin
        P.next_word >>= fun n ->
        P.read_register r >>= fun w ->
        P.Return (Target.Offset Word.(n + w))
      end
    | Push -> P.Return (Target.Push)
    | Pop -> P.Return (Target.Pop)
    | Peek -> P.read_special Special.PC >>= fun w -> P.Return (Target.Offset w)
    | Pick -> begin
        P.next_word >>= fun n ->
        P.read_special Special.SP >>= fun sp ->
        P.Return (Target.Offset Word.(n + sp))
      end
    | SP -> P.Return (Target.Special Special.SP)
    | PC -> P.Return (Target.Special Special.PC)
    | EX -> P.Return (Target.Special Special.EX)
    | Direct -> P.next_word >>= fun n -> P.Return (Target.Value n)
    | Indirect -> P.next_word >>= fun n -> P.Return (Target.Offset n)
    | Literal v -> P.Return (Target.Value v)
  end
