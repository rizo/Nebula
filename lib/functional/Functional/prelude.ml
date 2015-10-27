let ( @ ) f g =
  fun x -> f (g x)

let const a b =
  a

let id a =
  a

let enum_from_to l h =
  let rec loop accum n =
    if n >= h then List.rev accum
    else loop (n :: accum) (n + 1)
  in
  loop [] l

type ('a, 'b) either =
  | Left of 'a
  | Right of 'b

module Int = struct
  type t = int

  let compare x y =
    compare x y
end

module Float = struct
  type t = float

  let compare x y =
    compare x y
end

module Exception = struct
  type t = exn
end
