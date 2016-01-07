module String_map = Map.Make (String)

type t = {
  pc : Word.t;
  encoded : Word.t list;
  labels : Word.t String_map.t;
}

let beginning_at pc =
  { pc; encoded = []; labels = String_map.empty }
