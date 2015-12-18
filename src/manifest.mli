(** Manifest of simulated hardware devices.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

(** The manifest. *)
type t

exception No_such_device of device_index

module Record : sig
  type t = {
    index : device_index;
    device : Device.t
  }
end

(** The empty manifest. *)
val empty : t

(** Register a new device in the manifest.

    Registered devices are given an index that is unique to the manifest. *)
val register : Device.t -> t -> t

(** Update the record of an existing device.

    If the record index does not exist in the manifest, then the manifest is
    unchanged. *)
val update : Record.t -> t -> t

(** Retrieve all registered devices. *)
val all : t -> Record.t list

(** Retrieve a manifest record by its assigned index.

    Assumes the provided index has been assigned. Raises {! No_such_device} if
    this is not the cause. *)
val get_record : device_index -> t -> Record.t

(** Shortcut to query a device's information based on its assigned index. *)
val query : device_index -> t -> Device.Info.t

(** The number of devices in the manifest. *)
val size : t -> int
