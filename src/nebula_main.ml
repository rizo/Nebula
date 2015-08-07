open Printf

open IO.Monad
open Prelude

let show_error_and_exit ?computer message =
  IO.put_string ("ERROR: " ^ message ^ "\n") >>= fun () ->

  (match computer with
  | Some c -> IO.put_string ("\n" ^ Computer.show c)
  | None -> IO.unit ()) >>= fun () ->

  IO.terminate 1

let clock_instance clock =
  (module struct
    module Device = Clock

     let this = clock
   end : Device.Instance)

let () =
  IO.main begin
    let open Computer in
    let open Device_set in

    Mem.of_file "a.bin" >>= function
    | Left (`Bad_memory_file message) -> show_error_and_exit ("Reading memory file: " ^ message)
    | Right memory ->
      begin
        Device_set.make >>= fun devices ->
        let manifest =
          Manifest.empty
          |> Manifest.register (clock_instance devices.clock)
        in
        Engine.launch devices { Computer.default with memory; manifest } >>= fun error ->

        match error with
        | Engine.Bad_decoding (w, computer) ->
          begin
            show_error_and_exit
              (sprintf "Failed to decode word %s." (Word.show w)) ~computer
          end
        | Engine.Stack_overflow computer -> show_error_and_exit "Stack overflow." ~computer
        | Engine.Stack_underflow computer -> show_error_and_exit "Stack underflow" ~computer
        | _ -> show_error_and_exit
                 (sprintf "Unexpected failure: %s" (Printexc.to_string error))
      end
  end
