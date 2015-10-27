open Common

open OUnit2

let suit =
  let open Computer in
  let open Computer_state in
  let open Computer_state.Monad in
  let open Instruction in

  "instruction" >::: [
    "SET" >:: (fun ctx ->
        let cpu_before = Cpu.write_register Reg.Y (word 42) Cpu.empty in
        let cpu_expected = Cpu.write_register Reg.X (word 42) cpu_before in
        let cpu =
          execute (Binary (Code.Set, Address.Reg_direct Reg.X, Address.Reg_direct Reg.Y))
          |> run { Computer.default with cpu = cpu_before }
          |> fst
          |> fun c -> c.cpu
        in
        assert_equal cpu cpu_expected)
  ]
