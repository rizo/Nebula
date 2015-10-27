open OUnit2

open Common
open Spec

let suite =
  let open Address in
  let open Computer in
  let open Computer_state.Monad in
  let open Reg in
  let open Special in

  "address" >::: [
    "register direct" >:: (fun ctx -> todo "New addressing implementation.");

    "register indirect" >:: (fun ctx -> todo "New addressing implementation.");

    "register indirect offset" >:: (fun ctx -> todo "New addressing implementation.");

    "peek" >:: (fun ctx -> todo "New addressing implementation.");

    "pick" >:: (fun ctx -> todo "New addressing implementation.");

    "sp" >:: (fun ctx -> todo "New addressing implementation.");

    "pc" >:: (fun ctx -> todo "New addressing implementation.");

    "ex" >:: (fun ctx -> todo "New addressing implementation.");

    "direct" >:: (fun ctx -> todo "New addressing implementation.");

    "indirect" >:: (fun ctx -> todo "New addressing implementation.");

    "literal" >:: (fun ctx -> todo "New addressing implementation.");
  ]
