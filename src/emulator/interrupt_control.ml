(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

include Functional

type t = {
  dequeuing : bool;
  queue : Interrupt.t Persistent_queue.t;
  trigger : Interrupt.Trigger.t option;
}

let max_queued_interrupts =
  256

exception Caught_fire

let empty = {
  dequeuing = true;
  queue = Persistent_queue.empty;
  trigger = None
}

let enqueue interrupt t =
  if Persistent_queue.size t.queue = max_queued_interrupts then
    raise Caught_fire
  else
    { t with queue = Persistent_queue.push interrupt t.queue }

let trigger tri t =
  { t with trigger = Some tri }

let triggered t =
  t.trigger
  |> Option.Functor.map (fun tri -> (tri, { t with trigger = None }))

let handle t =
  if t.dequeuing then
    Persistent_queue.pop t.queue
    |> Option.Functor.map (fun (interrupt, q) -> (interrupt, { t with queue = q }))
  else None

let dequeuing t =
  t.dequeuing

let enable_dequeuing t =
  { t with dequeuing = true }

let disable_dequeuing t =
  { t with dequeuing = false }
