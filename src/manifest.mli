open Prelude

type t

val empty : t

val register : (module Device.Instance) -> t -> t

val query : word -> t -> Device.Info.t

val size : t -> int
