open OUnit2

open Prelude

let suite =
  let open Mem in
  "memory" >::: [
    "initializes to zero" >:: (fun ctx ->
        let m = empty in

        for i = 0 to 0x10000 do
          assert_equal (read (word i) m) (word 0)
        done);

    "allows reading and writing to arbitrary locations" >:: (fun ctx ->
        let m = empty in
        let locations = List.map word [2; 0xdead; 42; 200] in
        let value = word 10 in

        locations |> List.iter (fun offset ->
            assert_equal
              (m |> write offset value |> read offset)
              value));
  ]
