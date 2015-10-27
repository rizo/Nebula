open Common

let registers =
  let open Reg in
  [ A; B; C; X; Y; Z; I; J ]

let specials =
  let open Special in
  [ EX; SP; PC; IA ]

let run_program ?(computer = Computer.default) t =
  t
  |> Computer_state.of_program
  |> Computer_state.run computer
