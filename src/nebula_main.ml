open Printf

open IO.Monad
open Prelude

let show_error_and_exit ?computer message =
  IO.put_string ("ERROR: " ^ message ^ "\n") >>= fun () ->

  (match computer with
  | Some c -> IO.put_string ("\n" ^ Computer.show c)
  | None -> IO.unit ()) >>= fun () ->

  IO.terminate 1

let () =
  IO.main begin
    let open Computer in

    Mem.of_file "a.bin" >>= function
    | Left (`Bad_memory_file message) ->
      show_error_and_exit ("Reading memory file: " ^ message)

    | Right memory ->
      try
        Engine.launch { Computer.default with memory }
      with
      | Engine.Bad_decoding (w, computer) ->
        show_error_and_exit
          (sprintf "Failed to decode word %s." (Word.show w)) ~computer

      | Engine.Stack_overflow computer ->
        show_error_and_exit "Stack overflow." ~computer

      | Engine.Stack_underflow computer ->
        show_error_and_exit "Stack underflow" ~computer
  end
