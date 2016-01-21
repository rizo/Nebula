(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

module String_map = Map.Make (String)

type label_map = Label_loc.t String_map.t

type t = {
  pc : Word.t;
  encoded : Assembled.t list;
  labels : label_map;
}

let beginning_at pc =
  { pc; encoded = []; labels = String_map.empty }

let labels_list t =
  String_map.bindings t.labels

let set_label_loc name loc t =
  let updated_labels =
    String_map.add name loc t.labels
  in
  { t with labels = updated_labels }

let lookup_label_loc name t =
  try
    Some (String_map.find name t.labels)
  with Not_found -> None
