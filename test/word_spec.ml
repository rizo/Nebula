open Common

open Properties.Dsl

let gen_bounded_int =
  Gen.choose_int 0 (0xffff + 1)

let gen_word =
  gen_bounded_int |> Gen.Functor.map word

let suite =
  group "word" [
    for_all ~label:"integral conversion in valid range" ~show:show_int
      gen_bounded_int
      (fun i -> Word.to_int (word i) = i);

    for_all ~label:"discards beyond the word size on conversion" ~show:show_int
      (Gen.choose_int 0x10000 max_int)
      (fun i ->
         let lower_part = i land 0xffff in
         Word.to_int (word i) = lower_part);

    check ~label:"addition"
      (fun () -> Word.(word 3 + word 2) = word 5);

    check ~label:"subtraction"
      (fun () -> Word.(word 5 - word 3) = word 2);

    check ~label:"overflow cycle"
      (fun () -> Word.(word 0xfffe + word 2) = word 0);

    check ~label:"underflow cycle"
      (fun () -> Word.(word 1 - word 3) = word 0xfffe);

    check ~label:"comparison"
      (fun () ->
         Word.(compare (word 4) (word 10)) < 0 &&
         Word.(compare (word 200) (word 0)) > 0 &&
         Word.(compare (word 5) (word 5) = 0));

    check ~label:"hexadecimal literal"
      (fun () ->
         Word.show (word 2) = "0x0002" &&
         Word.show (word 0xdead) = "0xdead");
  ]
