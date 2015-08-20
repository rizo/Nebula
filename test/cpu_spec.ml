open OUnit2

open Prelude

let suite =
  let open Cpu in

  "cpu" >::: [
    "registers are initialized to zero" >:: (fun ctx ->
        assert_equal
          (List.for_all (fun r -> read_register r empty = word 0) Spec.registers)
          true);

    "EX is initialized to zero" >:: (fun ctx ->
        assert_equal (read_special Special.EX empty) (word 0));

    "PC is initialized to zero" >:: (fun ctx ->
        assert_equal (read_special Special.PC empty) (word 0));

    "IA is initialized to zero" >:: (fun ctx ->
        assert_equal (read_special Special.IA empty) (word 0));

    "SP is initialized to zero" >:: (fun ctx ->
        assert_equal (read_special Special.SP empty) (word 0));

    "registers can be read and written to" >:: (fun ctx ->
        let test_value = word 42 in
        List.iter (fun reg ->
            assert_equal (empty |> write_register reg test_value |> read_register reg) test_value)
          Spec.registers);

    "special registers can be read and written to" >:: (fun ctx ->
        let test_value = word 0xab41 in
        List.iter (fun s ->
            assert_equal (empty |> write_special s test_value |> read_special s) test_value)
          Spec.specials);
  ]
