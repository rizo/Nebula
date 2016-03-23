(** Common definitions.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

(** Convenient alias for {! Word.t } *)
type word = Word.t

(** Convenient alias for {! Word.of_int }. *)
val word : int -> word

(** Simulated devices are each given a unique index. *)
type device_index = word

(** The contents of an interrupt. *)
type message = word

module Time_stamp : sig
  type t = int64

  val ( - ) : t -> t -> t
end
