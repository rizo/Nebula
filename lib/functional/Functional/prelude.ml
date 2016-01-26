let ( @ ) f g =
  fun x -> f (g x)

let const a b =
  a

let id a =
  a

let list_of_options ms =
  let rec go accum = function
    | [] -> accum
    | Some x :: xs -> go (x :: accum) xs
    | None :: xs -> go accum xs
  in
  go [] ms

let enum_from_to l h =
  let rec loop accum n =
    if n >= h then List.rev accum
    else loop (n :: accum) (n + 1)
  in
  loop [] l

let partition_map p ts =
  let rec go matching other = function
    | [] -> (matching, other)
    | t :: ts -> begin
        match p t with
        | Some a -> go (a :: matching) other ts
        | None -> go matching (t :: other) ts
      end
  in
  go [] [] ts

let filter_map p ts =
  let rec go keep = function
    | [] -> keep
    | t :: ts -> match p t with Some a -> go (a :: keep) ts | None -> go keep ts
  in
  go [] ts

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
