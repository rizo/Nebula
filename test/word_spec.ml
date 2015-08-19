open OUnit2

open Prelude

let suite =
  let open Word in

  "word" >::: [
    "converts from an integer within the word size" >:: (fun ctx ->
        assert_equal (to_int (word 2)) 2);

    "converts to an integer" >:: (fun ctx ->
        assert_equal (to_int (word 45)) 45;
        assert_equal (to_int (word 0)) 0;
        assert_equal (to_int (word 0xffff)) 0xffff);

    "discards anything above the word size on conversion" >:: (fun ctx ->
        let w = word 0x12345678 in
        assert_equal (to_int w) 0x5678);

    "supports addition" >:: (fun ctx ->
        assert_equal (word 3 + word 2) (word 5));

    "supports subtration" >:: (fun ctx ->
        assert_equal (word 5 - word 3) (word 2));

    "cycles on overflow" >:: (fun ctx ->
        assert_equal (word 0xfffe + word 2) (word 0));

    "cycles on underflow" >:: (fun ctx ->
        assert_equal (word 1 - word 3) (word 0xfffe));

    "compares" >:: (fun ctx ->
        assert_equal (Word.(compare (word 4) (word 10)) < 0) true;
        assert_equal (Word.(compare (word 200) (word 0)) > 0) true;
        assert_equal (Word.(compare (word 5) (word 5)) = 0) true);

    "displays as a hexadecimal literal" >:: (fun ctx ->
        assert_equal (show (word 2)) "0x0002";
        assert_equal (show (word 0xdead)) "0xdead");
  ]
