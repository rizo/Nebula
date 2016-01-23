open Functional.Prelude

module M = Machine

module String_map = Map.Make (String)

type t =
  | Constant of int
  | Add of t * t
  | Var of string
  | Let of string * t * t

module Table = struct
  type t = int String_map.t

  let update t =
    String_map.map (fun v -> v + 1) t

  let look_up name t =
    String_map.find name t

  let add name t =
    t
    |> update
    |> String_map.add name 0
end

let compile t =
  let module S = String_map in

  let rec loop table t =
    let open Machine.Monad in

    match t with
    | Constant x -> M.push x
    | Let (name, value, body) -> begin
        loop table value >>= fun () ->
        loop (Table.add name table) body >>= fun () ->
        M.swap >>= fun () ->
        M.pop
      end
    | Var name -> M.peek (Table.look_up name table)
    | Add (lhs, rhs) -> begin
        loop table lhs >>= fun () ->
        loop (Table.update table) rhs >>= fun () ->
        M.apply M.Arithmetic.Add
      end
  in
  loop S.empty t
