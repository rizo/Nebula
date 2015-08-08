(** Helper for invoking ocamlbuild. *)

let package_depends =
  [ "cmdliner";
    "ctypes";
    "ctypes.foreign";
    "ppx_deriving.std";
    "threads" ]

let test_package_depends =
  [ "oUnit" ]

let tags =
  [ "thread" ]

let external_libaries =
  [ "SDL2"]

let compiler_flags =
  [ "-cflags -bin-annot,-safe-string" ]

module Command : sig
  type t

  val run_and_exit : t -> unit

  val of_list : string list -> t

  val append : t -> t -> t
end = struct
  type t = string list

  let run_and_exit t =
    let rendered = String.concat " " t in
    print_endline rendered;
    let status = Sys.command rendered in
    exit status

  let of_list parts =
    parts

  let append extra t =
    List.append t extra
end

module Command_map = Map.Make(String)

let commands =
  let ocamlbuild_command =
    Command.of_list [ "ocamlbuild"; "-I src"; "-use-ocamlfind" ]
    |> Command.append (Command.of_list compiler_flags)
  in
  let package_flags = List.map (fun p -> "-pkg " ^ p) package_depends in
  let tag_flags = List.map (fun t -> "-tag " ^ t) tags in
  let external_library_flags =
    let linker_flags = List.map (fun l -> "-cclib,-l" ^ l) external_libaries in
    "-lflags" :: linker_flags
  in

  let open Command in

  let lib_command =
    ocamlbuild_command
    |> append (of_list package_flags)
    |> append (of_list tag_flags)
    |> append (of_list external_library_flags)
    |> append (of_list [ "nebula.cmxa"; "nebula.cma" ])
  in

  let top_command =
    ocamlbuild_command
    |> append (of_list package_flags)
    |> append (of_list tag_flags)
    |> append (of_list external_library_flags)
    |> append (of_list ["-pkg utop"])
    |> append (of_list [ "nebula.top" ])
  in

  let doc_command =
    let doc_package_flags = "-package " ^ (String.concat "," ("utop" :: package_depends)) in
    let open Command in
    of_list ["ocamlfind"; "ocamldoc"; "-thread"]
    |> append (of_list [doc_package_flags])
    |> append (of_list [
      "-I _build/src";
      "-d doc";
      "-keep-code";
      "-html";
      "src/*.mli";
      "src/*.ml" ])
  in

  let test_command =
    let test_package_flags = List.map (fun p -> "-pkg " ^ p) test_package_depends in

    ocamlbuild_command
    |> append (of_list [ "-I test" ])
    |> append (of_list tag_flags)
    |> append (of_list package_flags)
    |> append (of_list test_package_flags)
    |> append (of_list [ "nebula_test.byte" ])
  in

  let nebula_command =
    ocamlbuild_command
    |> append (of_list package_flags)
    |> append (of_list tag_flags)
    |> append (of_list external_library_flags)
    |> append (of_list [ "nebula_main.native" ])
  in

  let open Command_map in
  empty
  |> add "lib" lib_command
  |> add "top" top_command
  |> add "doc" doc_command
  |> add "nebula" nebula_command
  |> add "test" test_command

let lookup_target name =
  if Command_map.mem name commands then
    Some (Command_map.find name commands)
  else None

let exit_with_error message =
  print_endline message;
  exit 1

let () =
  let args = Sys.argv in

  if Array.length args != 2 then
    exit_with_error "Expected a target."
  else begin
    let target = args.(1) in

    match lookup_target target with
    | Some command -> Command.run_and_exit command
    | None -> exit_with_error ("No such target: " ^ target)
  end
