(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Prelude

module Block = struct
  type 'a t =
    | Next of (unit -> 'a)
    | Exception of exn

  let map f = function
    | Next n -> Next (fun () -> f (n ()))
    | Exception e -> Exception e
end

module F = Free.Make (Block)

type 'a t = 'a F.t

let throw exn =
  F.lift (Block.Exception exn)

let rec catch t recovery =
  match t with
  | F.Suspend (Block.Exception exn) -> recovery exn
  | F.Suspend (Block.Next n) -> F.Suspend (Block.map (fun b -> catch b recovery) (Block.Next n))
  | F.Return _ as r -> r

module Monad_instance = F.Monad_instance

module Monad = F.Monad

module Functor_instance = F.Functor_instance

module Functor = F.Functor

let unit a =
  F.Return a

(** Does not interpret OCaml exceptions. *)
let unsafe_lift f =
  F.Suspend (Block.map (fun a -> F.Return a) (Block.Next f))

let lift f =
  let open Monad in
  unsafe_lift (fun () -> Or_exn.protect f ()) >>= begin function
    | Left error -> throw error
    | Right v -> unit v
  end

let rec unsafe_perform = function
  | F.Return a -> a
  | F.Suspend (Block.Next n) -> unsafe_perform (n ())
  | F.Suspend (Block.Exception e) -> raise e

let main t =
  unsafe_perform t |> ignore

let terminate status =
  lift (fun () -> exit status)

let put_string s =
  lift (fun () -> print_string s)
