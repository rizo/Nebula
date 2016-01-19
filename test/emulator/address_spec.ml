open OUnit2

open Common
open Spec

module C = Computer

let suite =
  let open Computer.Monad in

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
