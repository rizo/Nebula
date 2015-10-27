(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

open Functional

module F = Option.Functor

type context = A | B

let code w =
  let open Code in
  match Word.to_int w with
  | 0x01 -> Some Set
  | 0x02 -> Some Add
  | 0x03 -> Some Sub
  | 0x04 -> Some Mul
  | 0x05 -> Some Mli
  | 0x06 -> Some Div
  | 0x07 -> Some Dvi
  | 0x0b -> Some Bor
  | 0x0f -> Some Shl
  | 0x12 -> Some Ife
  | 0x13 -> Some Ifn
  | 0x14 -> Some Ifg
  | 0x16 -> Some Ifl
  | 0x1e -> Some Sti
  | _ -> None

let special_code w =
  let open Special_code in
  match Word.to_int w with
  | 0x01 -> Some Jsr
  | 0x08 -> Some Int
  | 0x09 -> Some Iag
  | 0x0a -> Some Ias
  | 0x0b -> Some Rfi
  | 0x0c -> Some Iaq
  | 0x10 -> Some Hwn
  | 0x11 -> Some Hwq
  | 0x12 -> Some Hwi
  | 0x1e -> Some Abt
  | 0x1f -> Some Dbg
  | _ -> None

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

let address ctx w =
  let i = Word.to_int w in

  if i <= 0x07 then
    register w |> F.map (fun r -> Address.Reg_direct r)
  else if (i >= 0x08) && (i < 0x0f) then
    register Word.(w - word 0x08) |> F.map (fun r -> Address.Reg_indirect r)
  else if (i >= 0x10) && (i <= 0x17) then
    register Word.(w - word 0x10) |> F.map (fun r -> Address.Reg_indirect_offset r)
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

let instruction w =
  let open Option.Monad in

  let binary w =
    code Word.(w land word 0x1f) >>= fun c ->
    address A Word.((w land word 0xfc00) lsr 10) >>= fun a ->
    address B Word.((w land word 0x3e0) lsr 5) >>= fun b ->
    Some (Instruction.Binary (c, b, a))
  in

  let unary w =
    special_code Word.((w land word 0x3e0) lsr 5) >>= fun c ->
    address A Word.((w land word 0xfc00) lsr 10) >>= fun a ->
    Some (Instruction.Unary (c, a))
  in

  match binary w with
  | Some ins -> Some ins
  | None -> unary w
