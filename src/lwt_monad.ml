include Monad_class.Extend(
  struct
    type 'a t = 'a Lwt.t

    let pure a =
      Lwt.return (Lazy.force a)

    let bind f ma =
      Lwt.(ma >>= f)
  end)
