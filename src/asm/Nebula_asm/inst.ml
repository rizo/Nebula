module Code = struct
  type t =
    | Set
    | Add
    | Sub
    | Mul | Mli
    | Div | Dvi
    | Mod | Mdi
    | And
    | Bor
    | Xor
    | Shr
    | Asr
    | Shl
    | Ifb | Ifc | Ife | Ifn | Ifg | Ifa | Ifl | Ifu
    | Adx
    | Sbx
    | Sti
    | Std
end

module Special_code = struct
  type t =
    | Jsr
    | Int
    | Iag
    | Ias
    | Rfi
    | Iaq
    | Hwn
    | Hwq
    | Hwi
end

type t =
  | Unary : Special_code.t * (_, _) Value.t -> t
  | Binary : Code.t * (_, _) Value.t * (_, _) Value.t -> t
