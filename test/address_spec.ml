open OUnit2

open Prelude
open Spec

let suite =
  let open Address in
  let open Computer in
  let open Program.Monad in
  let open Reg in
  let open Special in

  let set_get ?(computer = Computer.default) addr =
    (set (word 42) addr >>= fun () -> get addr) |> run ~computer in

  "address" >::: [
    "register direct" >:: (fun ctx ->
        let (_, w) = set_get (Reg_direct Reg.X) in
        assert_equal w (word 42));

    "register indirect" >:: (fun ctx ->
        let computer = {
          Computer.default with cpu = Cpu.write_register Reg.B (word 5) Cpu.empty
        } in
        let (c, w) = set_get ~computer (Reg_indirect Reg.B) in
        assert_equal w (word 42);
        assert_equal (Mem.read (word 5) c.memory) (word 42));

    "register indirect offset" >:: (fun ctx ->
        let computer = {
          Computer.default with
          cpu = Cpu.empty |> Cpu.write_register I (word 10) |> Cpu.write_special PC (word 2);
          memory = Mem.empty |> Mem.write (word 2) (word 40)
        } in
        let addr = Reg_indirect_offset I in

        let (c, ()) = set (word 42) addr |> run ~computer in
        assert_equal (Mem.read (word 50) c.memory) (word 42);

        let (_, w) =
          get addr
          |> run ~computer:{ c with cpu = Cpu.write_special PC (word 2) c.cpu }
        in
        assert_equal w (word 42));

    "peek" >:: (fun ctx ->
        let (_, w) = set_get Peek in
        assert_equal w (word 42));

    "pick" >:: (fun ctx ->
        let computer = {
          Computer.default with
          cpu = Cpu.empty |> Cpu.write_special PC (word 7) |> Cpu.write_special SP (word 0xfffe);
          memory = Mem.empty |> Mem.write (word 7) (word 1)
        } in
        let (c, ()) = set (word 42) Pick |> run ~computer in
        assert_equal (Mem.read (word 0xffff) c.memory) (word 42);

        let (_, w) =
          get Pick
          |> run ~computer:{ c with cpu = Cpu.write_special PC (word 7) c.cpu }
        in
        assert_equal w (word 42));

    "sp" >:: (fun ctx ->
        let (_, w) = set_get SP in
        assert_equal w (word 42));

    "pc" >:: (fun ctx ->
      let (_, w) = set_get PC in
      assert_equal w (word 42));

    "ex" >:: (fun ctx ->
        let (_, w) = set_get EX in
        assert_equal w (word 42));

    "direct" >:: (fun ctx ->
        let computer = {
          Computer.default with
          cpu = Cpu.empty |> Cpu.write_special PC (word 6);
          memory = Mem.empty |> Mem.write (word 6) (word 42)
        } in
        let (_, w) = get Direct |> run ~computer in
        assert_equal w (word 42);

        let (c, ()) = set (word 20) Direct |> run ~computer in
        assert_equal { computer with cpu = Cpu.write_special PC (word 6) c.cpu } computer);

    "indirect" >:: (fun ctx ->
        let computer = {
          Computer.default with
          cpu = Cpu.empty |> Cpu.write_special PC (word 250);
          memory = Mem.empty |> Mem.write (word 250) (word 50);
        } in
        let (c, ()) = set (word 42) Indirect |> run ~computer in
        assert_equal (Mem.read (word 50) c.memory) (word 42);

        let (_, w) = get Indirect
                     |> run ~computer:{ c with cpu = Cpu.write_special PC (word 250) c.cpu } in
        assert_equal w (word 42));

    "literal" >:: (fun ctx ->
        let (c, ()) = set (word 42) (Literal (word 20)) |> run in
        assert_equal c Computer.default;

        let (_, w) = get (Literal (word 20)) |> run in
        assert_equal w (word 20));
  ]
