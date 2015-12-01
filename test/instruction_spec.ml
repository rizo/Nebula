open Common

open OUnit2

module C = Computer
module Cs = Computer_state
module I = Instruction

let suit =
  let open Computer.Monad in

  "instruction" >::: [
    "SET" >:: (fun ctx ->
        let cpu_before = Cpu.write_register Reg.Y (word 42) Cpu.empty in
        let cpu_expected = Cpu.write_register Reg.X (word 42) cpu_before in
        let cpu =
          I.execute (I.Binary (Code.Set, Address.Reg_direct Reg.X, Address.Reg_direct Reg.Y))
          |> C.run Cs.{ Cs.default with cpu = cpu_before }
          |> fst
          |> fun c -> c.Cs.cpu
        in
        assert_equal cpu cpu_expected)
  ]
