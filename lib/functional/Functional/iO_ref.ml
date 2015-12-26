(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

type 'a t = 'a ref

let make a =
  IO.unit (ref a)

let set aa t =
  IO.lift (fun () -> t := aa)

let get t =
  IO.lift (fun () -> !t)

let modify f t =
  IO.Monad.(get t >>= fun a -> set (f a) t)
