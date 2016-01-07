open Functional
open Functional.Prelude

module Inner = State.Make (Dcpu16_state)

module Error = struct
  type t =
    | Undefined_label of string
end

module Self = Either_trans.Make (Error) (Inner.Monad_instance)

include Self

let run pc t =
  run t |> Inner.run (Dcpu16_state.beginning_at pc)

let assemble ?(at = Word.of_int 0) t =
  run at t |> begin function
    | (_, Left e) -> Left e
    | (s, Right ()) -> Right s.Dcpu16_state.encoded
  end

let emit ws =
  lift @@ Inner.modify begin fun { Dcpu16_state.pc; encoded; labels } ->
    Dcpu16_state.{
      pc = Word.(pc + of_int (List.length ws));
      encoded = List.append encoded ws;
      labels;
    }
  end

let label =
  lift @@ Inner.gets (fun s -> Value.(A (I s.Dcpu16_state.pc)))
