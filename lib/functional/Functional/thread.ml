(** Concurrent execution.

    This module provides a mechanism for concurrency, but not parallelism.
    Multiple tasks can be logically executed together, but the run-time will
    only ever execute one task at a given time.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

module type S = sig
  type _ context

  type _ t

  val atomic : 'a context -> 'a t

  val interleave : 'a t -> 'b t -> ('a * 'b) t

  val gather : 'a t list -> 'a list t

  val gather_unit : 'a t list -> unit t

  val run : 'a t -> 'a context

  module Monad_instance : Monad_class.S with type 'a t = 'a t

  module Monad : module type of Monad_class.Extend (Monad_instance)

  module Functor_instance : Functor_class.S with type 'a t = 'a t

  module Functor : module type of Functor_class.Extend (Functor_instance)
end

module Make (M : Monad_class.S) : (S with type 'a context = 'a M.t) = struct
  module T = Functor_class.Of_monad (M)

  type 'a context = 'a M.t

  module Block = struct
    type 'a t = Next of 'a context

    let map f (Next n) =
      Next (T.map f n)
  end

  module F = Free.Make (Block)

  type 'a t = 'a F.t

  let atomic c =
    F.Suspend (Block.Next (T.map (fun a -> F.Return a) c))

  let rec run = function
    | F.Return a -> M.pure (lazy a)
    | F.Suspend (Block.Next n) -> M.bind run n

  let rec interleave t1 t2 =
    let module C = Monad_class.Extend (M) in

    match (t1, t2) with
    | (F.Return a, _) -> F.Monad.(t2 >>= fun b -> pure (lazy (a, b)))
    | (_, F.Return b) -> F.Monad.(t1 >>= fun a -> pure (lazy (a, b)))
    | (F.Suspend (Block.Next n1), F.Suspend (Block.Next n2)) -> begin
        F.Monad.(atomic n1 >>= fun a ->
                 atomic n2 >>= fun b ->
                 interleave a b)
      end

  let gather ts =
    List.fold_right
      F.Functor.(fun t accum -> interleave t accum |> map (fun (a, b) -> a :: b))
      ts
      (atomic (M.pure (lazy [])))

  let gather_unit ts =
    F.Monad.(gather ts >>= fun _ -> F.Return ())

  module Monad_instance = F.Monad_instance

  module Monad = F.Monad

  module Functor_instance = F.Functor_instance

  module Functor = F.Functor
end
