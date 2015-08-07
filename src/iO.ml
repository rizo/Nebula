module Block = struct
  type 'a t = unit -> 'a

  let map f t =
    fun () -> f (t ())
end

include Free.Make(Block)

module Functor_instance = Functor_class.Of_monad(Monad_instance)

module Functor = Functor_class.Extend(Functor_instance)

let unit a =
  lift (fun _ -> a)

let rec unsafe_perform = function
  | Suspend f -> unsafe_perform (f ())
  | Return x -> x

let put_string s =
  lift (fun _ -> print_string s)

let get_string =
  lift (fun _ -> read_line ())

let main t =
  ignore (unsafe_perform t)

let terminate status =
  lift (fun _ -> exit status)

let now =
  lift Unix.gettimeofday
