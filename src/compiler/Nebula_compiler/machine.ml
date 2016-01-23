open Functional
open Functional.Prelude

module Arithmetic = struct
  type t = Add | Multiply
end

module Op = struct
  type 'a t =
    | Push of int * 'a
    | Pop of 'a
    | Peek of int * 'a
    | Apply of Arithmetic.t * 'a
    | Swap of 'a

  let map f = function
    | Push (x, n) -> Push (x, f n)
    | Pop n -> Pop (f n)
    | Peek (i, n) -> Peek (i, f n)
    | Apply (a, n) -> Apply (a, f n)
    | Swap n -> Swap (f n)
end

module Self = Free.Make (Op)

include Self

let push x =
  lift (Op.Push (x, ()))

let pop =
  lift (Op.Pop ())

let peek i =
  lift (Op.Peek (i, ()))

let apply f =
  lift (Op.Apply (f, ()))

let swap =
  lift (Op.Swap ())

let to_string t =
  let rec go accum = function
    | Suspend (Op.Push (x, n)) -> go (("push " ^ string_of_int x) :: accum) n
    | Suspend (Op.Pop n) -> go ("pop" :: accum) n
    | Suspend (Op.Peek (i, n)) -> go (("peek " ^ string_of_int i) :: accum) n
    | Suspend (Op.Apply (f, n)) -> begin
        go
          ((match f with
           | Arithmetic.Add -> "add"
           | Arithmetic.Multiply -> "mul")
           :: accum)
          n
      end
    | Suspend (Op.Swap n) -> go ("swap" :: accum) n
    | Return () -> List.rev accum
  in
  go [] t
