open Functional

open Properties.Dsl

let run_io =
  IO.unsafe_perform

let suite =
  group "IO" [
    check ~label:"unit"
      (fun () -> IO.unit 2 |> run_io = 2);
  ]
