type t = {
  clock : Clock.t
}

let make =
  let open IO.Monad in
  Clock.make >>= fun clock ->
  IO.unit { clock }
