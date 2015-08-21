open OUnit2

open Prelude
open Spec

let suite =
  let open Program in
  let open Program.Monad in

  "program" >::: [
    "read and write memory" >:: (fun ctx ->
        let (_, w) = read_memory (word 2) |> run_program in
        assert_equal w (word 0);

        let (_, w) = begin
          write_memory (word 0xdead) (word 42) >>= fun () ->
          read_memory (word 0xdead)
          end |> run_program
        in
        assert_equal w (word 42));

    "read and write special registers" >:: (fun ctx ->
        let (_, w) = read_special Special.EX |> run_program in
        assert_equal w (word 0);

        let (_, w) = begin
          write_special Special.EX (word 5) >>= fun () ->
          read_special Special.EX
        end |> run_program
        in
        assert_equal w (word 5));

    "read and write register" >:: (fun ctx ->
        let (_, w) = read_register Reg.I |> run_program in
        assert_equal w (word 0);

        let (_, w) = begin
          write_register Reg.J (word 42) >>= fun () ->
          read_register Reg.J
        end |> run_program
        in
        assert_equal w (word 42));

    "increment the program counter" >:: (fun ctx ->
        let open Computer in
        let (c, pc) = next_word |> run_program in
        assert_equal pc (word 0);
        assert_equal (Cpu.read_special Special.PC c.cpu) (word 1));

    "push values onto the stack" >:: (fun ctx ->
        let open Computer in
        let (c, ()) = sequence_unit [
            push (word 1);
            push (word 2);
            push (word 3)
          ] |> run_program
        in
        assert_equal (Cpu.read_special Special.SP c.cpu) (word 0xfffd));

    "pop values from the stack" >:: (fun ctx ->
        let open Computer in
        let (_, items) = begin
          sequence_unit [
            push (word 1);
            push (word 2);
            push (word 3);
          ] >>= fun () -> sequence [
            pop;
            pop;
            pop;
          ] end |> run_program
        in
        assert_equal items [word 3; word 2; word 1]);
  ]
