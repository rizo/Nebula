(** A persistent (immutable) queue.

    Pushing and popping from the queue is ammortized to be constant time. *)

(** The queue. *)
type 'a t

(** The empty queue. *)
val empty : 'a t

(** Enqueue a new element at the back. *)
val push : 'a -> 'a t -> 'a t

(** Dequeue the element at the front.

    If the queue is empty, then return {! None } *)
val pop : 'a t -> ('a * 'a t) option

(** The number of elements currently enqueued. *)
val size : 'a t -> int

