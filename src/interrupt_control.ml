type t = {
  queuing : bool;
  queue : Interrupt.t Persistent_queue.t;
  trigger : Interrupt.Trigger.t option;
}

let max_queued_interrupts =
  256

exception Caught_fire

let empty = {
  queuing = false;
  queue = Persistent_queue.empty;
  trigger = None
}

let receive interrupt = function
  | { queuing = false; _ } as t -> t
  | { queue; _ } as t ->
    if Persistent_queue.size queue = max_queued_interrupts then
      raise Caught_fire
    else
      { t with queue = queue |> Persistent_queue.push interrupt }

let trigger tri t =
  { t with trigger = Some tri }

let triggered t =
  t.trigger
  |> Option.Functor.map (fun tri -> (tri, { t with trigger = None }))

let handle t =
  Persistent_queue.pop t.queue
  |> Option.Functor.map (fun (interrupt, q) -> (interrupt, { t with queue = q }))

let queuing t =
  t.queuing

let enable_queuing t =
  { t with queuing = true }

let disable_queuing t =
  { t with queuing = false }
