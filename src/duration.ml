(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

type t = int64

let of_nanoseconds x =
  if Int64.compare x 0L < 0 then 0L else x

let nanoseconds t =
  t

let (+) =
  Int64.add

let (-) x y =
  of_nanoseconds (Int64.sub x y)

let (=) x y =
  Int64.compare x y = 0

let (<>) x y =
  not (x = y)

let (<) x y =
  Int64.compare x y < 0

let (<=) x y =
  Int64.compare x y <= 0

let (>) x y =
  not (x <= y)

let (>=) x y =
  not (x < y)
