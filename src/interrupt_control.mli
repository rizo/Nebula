(** Pure handling of interrupts. *)

type t

(** The initial controller with no trigger and an empty queue.

    Queuing is initially disabled. *)
val empty : t

(** The maximum number of interrupts that can be queued before a fatal error. *)
val max_queued_interrupts : int

(** Raised when the number of interrupts exceeds {! max_queued_interrupts}. *)
exception Caught_fire

(** Enqueue a new interrupt from an external source.

    If queueing is disabled, then do nothing. *)
val enqueue : Interrupt.t -> t -> t

(** Remove the next received interrupt from the queue, if there is one. *)
val handle : t -> (Interrupt.t * t) option

(** Define a new interrupt trigger.

    Only one interrupt can be triggered at once. *)
val trigger : Interrupt.Trigger.t -> t -> t

(** The currently triggered interrupt, if there is one.

    This will also remove the trigger. *)
val triggered : t -> (Interrupt.Trigger.t * t) option

(** Whether or not interrupts are being dequeued during execution. *)
val dequeuing : t -> bool

(** Enable interrupt dequeuing. *)
val enable_dequeuing :  t -> t

(** Disable interrupt dequeuing. *)
val disable_dequeuing : t -> t
