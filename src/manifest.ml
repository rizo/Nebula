open Prelude

module Table = Map.Make(Word)

exception No_such_device of word

type t = {
  devices : (module Device.Instance) Table.t;
  next_index : word;
}

let empty = {
  devices = Table.empty;
  next_index = word 0;
}

let register device t = {
  devices = Table.add t.next_index device t.devices;
  next_index = Word.(t.next_index + word 1);
}

let query index t =
  let (module I : Device.Instance) = Table.find index t.devices in
  I.Device.info

let size t =
  Word.to_int t.next_index
