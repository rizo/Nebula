open Functional
open Functional.Prelude

open Properties.Dsl

exception Mock_exception

let suite =
  group "Or_exn" [
    check ~label:"protect"
      (fun () ->
         let f () = raise Mock_exception in
         match Or_exn.protect f () with
         | Left err -> true
         | Right _ -> false);
  ]
