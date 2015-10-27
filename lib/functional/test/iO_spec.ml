open Functional

open Properties.Dsl

let run_io =
  IO.unsafe_perform

let suite =
  group "IO" [
    for_all ~label:"unit"
      Gen.int
      (fun x -> IO.unit x |> run_io = x);
  ]
