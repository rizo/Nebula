open Functional
open Properties.Dsl

let suite =
  group "nebula" [
  ]

let () =
  IO.main (run_and_terminate suite)
