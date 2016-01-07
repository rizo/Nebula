module Code = struct
  type t =
    | Set
    | Add
    | Sub
end

module Special_code = struct
  type t =
    | Hwi
end

type t =
  | Unary : Special_code.t * (_, _) Value.t -> t
  | Binary : Code.t * (_, _) Value.t * (_, _) Value.t -> t
