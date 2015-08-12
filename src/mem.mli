(** Simulated volatile memory. *)

open Prelude

type t

(** {2 Creation} *)

(** Memory with all cells initialized to zero. *)
val empty : t

(** Memory initialized from a raw byte stream. *)
val of_bytes : Bytes.t -> t

(** Read memory from an image file specified through its name. *)
val of_file : string -> ([> `Bad_memory_file of string], t) either IO.t

(** {2 Reading and writing} *)

(** Read a single word of memory. *)
val read : word -> t -> word

(** Write a single word of memory. *)
val write : word -> word -> t -> t
