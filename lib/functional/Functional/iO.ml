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
  | F.Suspend (Block.Next n) -> begin
      F.Suspend (Block.map (fun b -> catch b recovery) (Block.Next n))
    end
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

let lift_async (f : (('a -> unit) -> unit)) =
  let open Monad in

  let result = ref None in
  f (fun a -> result := Some a);

  let rec go () =
    lift begin fun () ->
      match !result with
      | Some a -> result := None; unit a
      | None -> lift id >>= go >>= id
    end
  in
  go () >>= id

let gather ts =
  let open Monad in

  let rec loop results remaining =
    match remaining with
    | [] -> unit results
    | ts -> begin
        match List.find_all (function F.Suspend (Block.Exception _) -> true | _ -> false) ts with
        | F.Suspend (Block.Exception e) :: _ -> throw e
        | _ -> begin
            let (finished, remaining) =
              partition_map (function F.Return a -> Some a | _ -> None) ts
            in

            remaining
            |> filter_map (function F.Suspend (Block.Next n) -> Some (lift n) | _ -> None)
            |> sequence >>= fun nexts ->
            loop (List.append finished results) nexts
          end
      end
  in
  loop [] ts

let gather_unit ts =
  Monad.(gather ts >>= fun _ -> unit ())

let rec forever t =
  Monad.(t >>= fun () -> forever t)

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

let sleep ~seconds =
  let on_alarm f = Sys.set_signal Sys.sigalrm (Sys.Signal_handle f) in
  let alarm_action = lift_async (fun register -> on_alarm (fun _ -> register ())) in

  let set_alarm = lift begin fun () ->
      Unix.setitimer Unix.ITIMER_REAL Unix.{ it_value = seconds; it_interval = 0.0 }
    end
  in
  Monad.(set_alarm >>= fun _ -> alarm_action)
