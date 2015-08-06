type 'a t = {
  inbox : 'a list;
  outbox : 'a list;
  size : int;
}

let empty = {
  inbox = [];
  outbox = [];
  size = 0;
}

let push a t =
  { t with inbox = a :: t.inbox; size = t.size + 1 }

let rec pop = function
  | { inbox = []; outbox = [] } -> None
  | { outbox = []; inbox; _ } as t -> pop { t with outbox = List.rev inbox; inbox = [] }
  | { outbox = x :: xs; size } as t -> Some (x, {t with outbox = xs; size = size - 1 })

let size t =
  t.size
