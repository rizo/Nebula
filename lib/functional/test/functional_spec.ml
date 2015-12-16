open Functional

open Properties.Dsl

let suite =
  group "functional" [
    IO_spec.suite;
    Or_exn_spec.suite;
    Prelude_spec.suite;
  ]

let () =
  IO.main (run suite)
