open Functional
open Functional.Prelude

open Properties.Dsl

module Data_store = struct
  module Op = struct
    type 'a t =
      | Put of string * 'a
      | Get of (string -> 'a)

    let rec map f = function
      | Put (v, n) -> Put (v, f n)
      | Get n -> Get (fun v -> f (n v))
  end

  module F = Free.Make (Op)

  type 'a t = 'a F.t

  let put v =
    F.lift (Op.Put (v, ()))

  let get =
    F.lift (Op.Get id)
end

let run value t =
  let cell = ref value in

  let rec loop = function
    | Data_store.F.Suspend (Data_store.Op.Get f) -> loop (f !cell)
    | Data_store.F.Suspend (Data_store.Op.Put (v, n)) -> cell := v; loop n
    | Data_store.F.Return a -> a
  in
  loop t

let suite =
  let open Data_store.F.Functor in
  let open Data_store.F.Monad in

  let alpha_string =
    Gen.string_of_n 4 Gen.alpha
  in

  group "free" [
    for_all ~label:"return"
      Gen.int
      (fun a -> Data_store.F.Return a |> run "abc" = a);

    for_all ~label:"flatMap-map"
      Gen.int
      (fun a ->
         (Data_store.put "abc" >>= fun () -> Data_store.F.Return a)
         |> map (fun a -> a * 2)
         |> run "def"
            = a * 2);
  ]
