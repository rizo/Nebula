let known_shells =
  ["emulator"; "asm"]

let directory_name shell =
  "shells/" ^ shell

let shell_executable shell =
  "./nebula_" ^ shell ^ ".top"

let usage_message =
  "Usage: " ^ Sys.executable_name ^ " [SHELL-NAME]"

let _ =
  let args = Array.to_list Sys.argv in

  match args with
  | [ _; shell ] -> begin
      if List.mem shell known_shells then begin
        Sys.chdir (directory_name shell);
        exit (Sys.command (shell_executable shell))
      end
      else begin
        let shell_list =
          known_shells
          |> List.map (fun s -> "`" ^ s ^ "`")
          |> String.concat ", "
        in
        prerr_endline ("Invalid shell. Expected one of " ^ shell_list);
        exit 1
      end
    end
  | _ -> prerr_endline usage_message
