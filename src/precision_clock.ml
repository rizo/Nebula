module Foreign = struct
  external get_time : unit -> int = "nebula_precision_clock_get_time"
end

let get_time () =
  Lwt.wrap (fun () -> Foreign.get_time ())
