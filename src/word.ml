type t = Unsigned.UInt16.t

include Unsigned.UInt16.Infix

let of_int =
  Unsigned.UInt16.of_int

let to_int =
  Unsigned.UInt16.to_int

let compare =
  Unsigned.UInt16.compare

let show t =
  Printf.sprintf "0x%04x" (to_int t)

let pp fmt t =
  Format.pp_print_string fmt (show t)
