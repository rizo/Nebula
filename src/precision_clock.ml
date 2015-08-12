module Foreign = struct
  external get_time : unit -> int = "nebula_precision_clock_get_time"
end

let get_time =
  IO.lift (fun () -> Foreign.get_time ())
