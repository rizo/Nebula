open Functional

open Properties.Dsl

let suite =
  let open Gen.Monad in

  group "prelude" [
    group "enum_from_to" [
      check ~label:"simple"
        (fun () ->
           Prelude.enum_from_to 0 2 = [0; 1]);

      for_all ~label:"list size"
        Gen.(choose_int 0 10 >>= fun low ->
             choose_int 1 5 >>= fun step ->
             unit (low, step + low))
        (fun (low, high) -> Prelude.enum_from_to low high |> List.length = high - low);

      for_all ~label:"empty range"
        Gen.(non_negative_int >>= fun low ->
             non_negative_int |> where (fun high -> high < low) >>= fun high ->
             unit (low, high))
        (fun (low, high) -> Prelude.enum_from_to low high |> List.length = 0);
    ];

    for_all ~label:"const"
      Gen.(int >>= fun a ->
           int |> where (fun b -> b <> a) >>= fun b ->
           unit (a, b))
      (fun (a, b) -> Prelude.const a b = a);

    for_all ~label:"id"
      Gen.int
      (fun a -> Prelude.id a = a);
  ]
