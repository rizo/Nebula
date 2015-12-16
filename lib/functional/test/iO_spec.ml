open Functional

open Properties.Dsl

let run_io =
  IO.unsafe_perform

exception Mock_exception

let suite =
  group "IO" [
    for_all ~label:"unit"
      Gen.int
      (fun x -> IO.unit x |> run_io = x);

    check ~label:"throw-catch"
      (fun () ->
         let open IO.Monad in

         let failing_code = IO.unit 42 >>= fun _ -> IO.throw Mock_exception in

         IO.catch failing_code begin function
             | Mock_exception -> IO.unit 5
             | error -> IO.throw error
           end
         |> run_io = 5);

    check ~label:"lift"
      (fun () ->
         let r = IO.lift (fun () -> raise Mock_exception) in

         IO.catch r begin function
           | Mock_exception -> IO.unit true
           | _ -> IO.unit false
         end
         |> run_io);
  ]
