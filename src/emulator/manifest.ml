(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

module Table = Map.Make (Word)

exception No_such_device of word

module Record = struct
  type t = {
    index : device_index;
    device : Device.t
  }
end

type t = {
  records : Record.t Table.t;
  next_index : device_index;
}

let empty = {
  records = Table.empty;
  next_index = word 0
}

let register device t = {
  records = Table.add t.next_index Record.{ device; index = t.next_index } t.records;
  next_index = Word.(t.next_index + word 1)
}

let get_record index t =
  try
    Table.find index t.records
  with
  | Not_found -> raise (No_such_device index)

let update r t =
  if not (Table.mem r.Record.index t.records) then
    t
  else
    { t with records = Table.add r.Record.index r t.records }

let all t =
  t.records |> Table.bindings |> List.map snd

let query index t =
  (get_record index t).Record.device#info

let size t =
  Word.to_int t.next_index
