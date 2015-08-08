open Printf

open IO.Monad
open Prelude

let show_error_and_exit ?computer message =
  IO.put_string ("ERROR: " ^ message ^ "\n") >>= fun () ->

  (match computer with
  | Some c -> IO.put_string ("\n" ^ Computer.show c)
  | None -> IO.unit ()) >>= fun () ->

  IO.terminate 1

let main file_name =
  IO.main begin
    let open Computer in

    Mem.of_file file_name >>= function
    | Left (`Bad_memory_file message) -> show_error_and_exit ("Reading memory file " ^ message)
    | Right memory ->
      begin
        Clock.make >>= fun clock ->

        let manifest =
          Manifest.empty
          |> Manifest.register (module Clock) clock
          |> Manifest.register (module Clock) clock
          |> Manifest.register (module Clock) clock
        in
        Engine.launch { Computer.default with memory; manifest } >>= fun error ->

        match error with
        | Engine.Bad_decoding (w, computer) ->
          begin
            show_error_and_exit
              (sprintf "Failed to decode word %s." (Word.show w)) ~computer
          end
        | Engine.Stack_overflow computer -> show_error_and_exit "Stack overflow." ~computer
        | Engine.Stack_underflow computer -> show_error_and_exit "Stack underflow" ~computer
        | Engine.No_such_device (index, computer) ->
          begin
            show_error_and_exit
              (sprintf "Index %s is not associated with a device." (Word.show index))
              ~computer
          end
        | _ -> show_error_and_exit
                 (sprintf "Unexpected failure: %s" (Printexc.to_string error))
      end
  end

let () =
  match Cli.parse_to main with
  | `Error _ -> exit 1
  | _ -> exit 0
