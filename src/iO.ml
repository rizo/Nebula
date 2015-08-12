module Block = struct
  type 'a t = (unit -> 'a)

  let map f t =
    fun () -> f (t ())
end

include Free.Make(Block)

let unit a =
  lift (fun () -> a)

let rec unsafe_perform = function
  | Return a -> a
  | Suspend t -> unsafe_perform (t ())

let main t =
  unsafe_perform t |> ignore

let terminate status =
  lift (fun () -> exit status)

let put_string s =
  lift (fun () -> print_string s)
