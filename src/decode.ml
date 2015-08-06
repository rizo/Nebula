(** Decode binary data for the processor.

    Generally, decoding errors will result in a value of {! None}. *)

open Option.Functor
open Prelude

(** The address context.

    In an instruction like

    {[
      ADD X, [Y]
    ]}

    the [A] context is [[Y]] and the [B] context is [X]. *)
type context = A | B

(** Decode an opcode. *)
let code w =
  let open Code in
  match Word.to_int w with
  | 0x01 -> Some Set
  | _ -> None

(** Decode a register. *)
let register w =
  match Word.to_int w with
  | 0 -> Some Reg.A
  | 1 -> Some Reg.B
  | 2 -> Some Reg.C
  | 3 -> Some Reg.X
  | 4 -> Some Reg.Y
  | 5 -> Some Reg.Z
  | 6 -> Some Reg.I
  | 7 -> Some Reg.J
  | _ -> None

(** Decode an address. *)
let address ctx w =
  let i = Word.to_int w in

  if i <= 0x07 then
    register w |> map (fun r -> Address.Reg_direct r)
  else if (i >= 0x08) && (i < 0x0f) then
    register Word.(w - word 0x08) |> map (fun r -> Address.Reg_indirect r)
  else if (i >= 0x10) && (i <= 0x17) then
    register Word.(w - word 0x10) |> map (fun r -> Address.Reg_indirect_offset r)
  else match i with
    | 0x18 when ctx = A -> Some Address.Pop
    | 0x18 when ctx = B -> Some Address.Push
    | 0x19 -> Some Address.Peek
    | 0x1a -> Some Address.Pick
    | 0x1b -> Some Address.SP
    | 0x1c -> Some Address.PC
    | 0x1d -> Some Address.EX
    | 0x1e -> Some Address.Indirect
    | 0x1f -> Some Address.Direct
    | _ ->
      if (ctx = A) && (i >= 0x20) && (i <= 0x3f) then
        Some (Address.Literal Word.(w - word 0x21))
      else
        None

(** Decode an instruction. *)
let instruction w =
  let open Option.Monad in

  let binary w =
    code Word.(w land word 0x1f) >>= fun c ->
    address A Word.((w land word 0xfc00) lsr 10) >>= fun a ->
    address B Word.((w land word 0x3e0) lsr 5) >>= fun b ->
    Some (Instruction.Binary (c, b, a))
  in

  binary w
