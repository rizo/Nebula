open Prelude

exception Bad_decoding of word * Computer.t

exception Stack_overflow of Computer.t

exception Stack_underflow of Computer.t

let step t =
  let open Computer_state.Monad in

  try
    let s = Computer_state.of_program Program.next_word >>= fun w ->
      match Decode.instruction w with
      | None -> raise (Bad_decoding (w, t))
      | Some ins -> Instruction.execute ins
    in
    Computer_state.run t s |> fst
  with
  | Program.Stack_underflow -> raise (Stack_underflow t)
  | Program.Stack_overflow -> raise (Stack_overflow t)

let rec launch t =
  launch (step t)
