open Cmdliner

module Args = struct
  let file_name =
    let doc = "The file name of the DCPU-16 memory image to load." in
    Arg.(required & pos 0 (some string) None & info [] ~docv:"FILE" ~doc)
end

let info =
  let doc = "DCPU-16 emulator" in
  Term.info "nebula" ~version:"0.1.0" ~doc

let parse_to f =
  let cmd = Term.(pure f $ Args.file_name), info in
  Term.eval cmd
