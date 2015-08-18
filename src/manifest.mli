(** Manifest of simulated hardware devices. *)

open Prelude

type t

exception No_such_device of device_index

(** The empty manifest. *)
val empty : t

(** Register a new device.

    Given a device type, create a new device instance with a unique index in the
    manifest. *)
val register : (module Device.S with type t = 'a) -> 'a -> t -> t

(** Update the record of an existing device. *)
val update : (module Device.Instance) -> t -> t

(** Retrieve all registered devices. *)
val all : t -> (module Device.Instance) list

(** Retrieve a device instance by its assigned index.

    Assumes the provided index has been assigned. Raises {! No_such_device} if
    this is not the cause. *)
val instance : device_index -> t -> (module Device.Instance)

(** Shortcut to query a device's information based on its assigned index. *)
val query : device_index -> t -> Device.Info.t

(** The number of devices in the manifest. *)
val size : t -> int
