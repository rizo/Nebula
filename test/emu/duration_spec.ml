open OUnit2

let suite =
  let open Duration in

  "duration" >::: [
    "initialized from integral values" >:: (fun ctx ->
        assert_equal (of_nanoseconds 5L |> nanoseconds) 5L);

    "cannot be negative" >:: (fun ctx ->
        assert_equal (of_nanoseconds (-3L) |> nanoseconds) 0L);

    "addition" >:: (fun ctx ->
        let d = Duration.(of_nanoseconds 5L + of_nanoseconds 8L) in
        assert_equal (nanoseconds d) 13L);

    "subtraction" >:: (fun ctx ->
        let d1 = Duration.(of_nanoseconds 10L - of_nanoseconds 3L) in
        assert_equal (nanoseconds d1) 7L;

        let d2 = Duration.(of_nanoseconds 3L - of_nanoseconds 8L) in
        assert_equal (nanoseconds d2) 0L);

    "comparison" >:: (fun ctx ->
        assert_equal Duration.(of_nanoseconds 5L = of_nanoseconds 5L) true;
        assert_equal Duration.(of_nanoseconds 5L <> of_nanoseconds 4L) true;
        assert_equal Duration.(of_nanoseconds 3L < of_nanoseconds 4L) true;
        assert_equal Duration.(of_nanoseconds 7L <= of_nanoseconds 7L) true;
        assert_equal Duration.(of_nanoseconds 8L > of_nanoseconds 3L) true;
        assert_equal Duration.(of_nanoseconds 8L >= of_nanoseconds 8L) true);
  ]
