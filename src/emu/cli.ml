(** Command-line argument processing helpers.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Cmdliner

module Args = struct
  let file_name =
    let doc = "The file name of the DCPU-16 memory image to load." in
    Arg.(required & pos 0 (some string) None & info [] ~docv:"FILE" ~doc)

  let start_interactive =
    let doc = "Start the emulator in interactive mode." in
    Arg.(value & flag & info ["s"; "start-interactive"] ~doc)
end

let info =
  let doc = "DCPU-16 emulator" in
  Term.info "nebula" ~version:"1.0.0" ~doc

let parse_to f =
  let cmd = Term.(const f $ Args.file_name $ Args.start_interactive), info in
  Term.eval cmd
