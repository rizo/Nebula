open Functional
open Properties.Dsl

let suite =
  group "nebula" [
    Word_spec.suite;
  ]

let () =
  IO.main (run_and_terminate suite)
