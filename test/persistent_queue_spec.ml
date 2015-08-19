open OUnit2

let suite =
  let open Persistent_queue in
  
  "persistent queue" >::: [
    "starts out empty" >:: (fun ctx ->
        let q = empty in
        assert_equal (size q) 0;
        assert_equal (pop q) None);

    "supports enqueing" >:: (fun ctx ->
        let q = empty |> push 1 in
        assert_equal (size q) 1;

        let q = q |> push 2 |> push 3 in
        assert_equal (size q) 3);

    "supports dequeing" >:: (fun ctx ->
        let q = empty |> push 1 |> push 2 |> push 3 in
        match pop q with
        | None -> assert_failure "Failed to pop!"

        | Some (x, q) ->
          assert_equal x 1;
          assert_equal (size q) 2);
  ]
