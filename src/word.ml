open Unsigned

type t = UInt16.t

include UInt16.Infix

let of_int =
  UInt16.of_int

let to_int =
  UInt16.to_int

let to_dword t =
  UInt32.of_int (to_int t)

let of_dword d =
  of_int (UInt32.to_int d)

let to_bool w = to_int w |> function
  | 0 -> false
  | _ -> true

let compare =
  Unsigned.UInt16.compare

let show t =
  Printf.sprintf "0x%04x" (to_int t)

let pp fmt t =
  Format.pp_print_string fmt (show t)
