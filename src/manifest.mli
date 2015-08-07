open Prelude

type t

exception No_such_device of device_index

val empty : t

val register : (module Device.S with type t = 'a) -> 'a -> t -> t

(* val register : (module Device.Instance) -> t -> t *)

val update : (module Device.Instance) -> t -> t

val all : t -> (module Device.Instance) list

val instance : device_index -> t -> (module Device.Instance)

val query : device_index -> t -> Device.Info.t

val size : t -> int
