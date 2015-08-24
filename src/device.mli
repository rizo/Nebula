(** Simulated hardware device specification.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Functional
open Prelude

open Unsigned

(** Identifying information for the device. *)
module Info : sig
    type t = {
    id : word * word;
    manufacturer : word * word;
    version : word
  }
end

(** User input data that is necessary for devices to function. *)
module Input : sig

  (** Input state. *)
  type t = {
    key_code : uint8 option; (** Key code of the last key pressed by the user. *)
  }

  (** The default (empty) input state. *)
  val none : t
end

(** Common device interface. *)
module type S = sig

  (** The device. *)
  type t

  (** Invoked by the {! Engine} when the device receives an interrupt. *)
  val on_interrupt : t -> t Program.t IO.t

  (** Invoked by the {! Engine} on every iteration of the event loop which
      renders graphics and handles user input. *)
  val on_interaction : Input.t -> Mem.t -> t -> t IO.t

  (** Invoked by the {! Engine} on every instruction that is decoded from memory
      and executed.

      The device can indicate here that it has generated an interrupt for the
      processor. *)
  val on_tick : t -> (t * (Interrupt.t option)) IO.t

  val info : Info.t
end

(** A specific device module and its instance as well as the reserved hardware
    index for the {! Manifest}. *)
module type Instance = sig
  module Device : S

  val this : Device.t

  val index : device_index
end

(** Construct a new device instance with an index. *)
val make_instance : (module S with type t = 'a) -> 'a -> device_index -> (module Instance)
