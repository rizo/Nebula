(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Prelude

module Table = Map.Make(Word)

exception No_such_device of word

type t = {
  instances : (module Device.Instance) Table.t;
  next_index : word;
}

let empty = {
  instances = Table.empty;
  next_index = word 0;
}

let register (type a) (module D : Device.S with type t = a) device t =
  let instance = Device.make_instance (module D) device t.next_index in
  {
    instances = Table.add t.next_index instance t.instances;
    next_index = Word.(t.next_index + word 1);
  }

let instance index t =
  try
    Table.find index t.instances
  with
  | Not_found -> raise (No_such_device index)

let update ((module I : Device.Instance) as instance) t =
  if not (Table.mem I.index t.instances) then
    t
  else
    { t with instances = Table.add I.index instance t.instances }

let all t =
  t.instances |> Table.bindings |> List.map snd

let query index t =
  let (module I : Device.Instance) = instance index t in
  I.Device.info

let size t =
  Word.to_int t.next_index
