type word = Word.t

let word =
  Word.of_int

type device_index = word

type message = word

let ( @ ) f g =
  fun x -> f (g x)

let const a b =
  a

let id a =
  a

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
