type 'a t =
  | Nil
  | Cons of 'a * 'a t Lazy.t

let head = function
  | Nil -> None
  | Cons (x, _) -> Some x

let tail = function
  | Nil -> Nil
  | Cons (_, xs) -> Lazy.force xs

let unit a =
  Cons (a, lazy Nil)

let append a = function
  | Nil -> unit a
  | Cons (x, xs) -> Cons (a, lazy (Cons (x, xs)))

let rec concat a b =
  match a with
  | Nil -> b
  | Cons (x, lazy xs) -> Cons (x, lazy (concat xs b))

let rec continually x =
  Cons (x, lazy (continually x))

let rec fold_right z f = function
  | Nil -> Lazy.force z
  | Cons (x, xs) -> f x (lazy (fold_right z f (Lazy.force xs)))

let exists f =
  fold_right (lazy false) (fun a b -> f a || (Lazy.force b))

let rec for_all f = function
  | Nil -> true
  | Cons (x, xs) -> f x && (for_all f (Lazy.force xs))

let rec take n t =
  if n <= 0 then Nil
  else
    match t with
    | Nil -> Nil
    | Cons (x, lazy xs) -> append x (take (n - 1) xs)

let rec take_while p = function
  | Nil -> Nil
  | Cons (x, xs) ->
    if p x then Cons (x, lazy (take_while p (Lazy.force xs)))
    else Nil

let rec drop n t =
  if n <= 0 then t
  else
    match t with
    | Nil -> Nil
    | Cons (_, lazy xs) -> drop (n - 1) xs

let rec drop_while p = function
  | Nil -> Nil
  | Cons (x, lazy xs) as t ->
    if p x then drop_while p xs
    else t

let rec map f = function
  | Nil -> Nil
  | Cons (x, lazy xs) -> Cons (f x, lazy (map f xs))

let filter f =
  fold_right (lazy Nil) (fun x xs ->
      if f x then Cons (x, xs)
      else Lazy.force xs)

let rec fill n x =
  if n <= 0 then Nil
  else
    Cons (x, lazy (fill (n - 1) x))

let of_list xs =
  List.fold_right append xs Nil

let to_list (t : 'a t) =
  fold_right (lazy []) (fun x (lazy xs) -> x :: xs) t

let rec iterate f z =
  Cons (z, lazy (iterate f (f z)))

let rec enum_from =
  iterate (fun x -> x + 1)

let rec enum_from_to low high =
  enum_from low |> take_while (fun x -> x <= high)

let rec zip_with f tx ty =
  match (tx, ty) with
  | (Nil, _) | (_, Nil) -> Nil
  | (Cons (x, xs), Cons (y, ys)) ->
    Cons ((f x y), lazy (zip_with f (Lazy.force xs) (Lazy.force ys)))

let zip (tx : 'a t) (ty : 'b t) =
  zip_with (fun x y -> (x, y)) tx ty

let rec iter f = function
  | Nil -> ()
  | Cons (x, xs) ->
    f x;
    iter f (Lazy.force xs)

let unfold s f =
  let rec loop s =
    match f s with
    | Some (s, a) -> Cons (a, lazy (loop s))
    | None -> Nil
  in
  loop s

let rec find p = function
  | Cons (a, tl) -> if p a then Some a else find p (Lazy.force tl)
  | Nil -> None

let reduce f = function
  | Cons (a, lt) -> begin
      Lazy.force lt
      |> fold_right (lazy a) f
      |> fun m -> Some m
    end
  | Nil -> None

let max t =
  t |> reduce (fun a (lazy m) -> if a > m then a else m)
