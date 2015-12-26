open Functional

open Properties.Dsl

let suite =
  group "functional" [
    Free_spec.suite;
    IO_spec.suite;
    Or_exn_spec.suite;
    Prelude_spec.suite;
  ]

let () =
  IO.main (run_and_terminate suite)
