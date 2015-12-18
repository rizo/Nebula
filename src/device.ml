(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

open Functional

open Unsigned

(** Identifying information for the device. *)
module Info = struct
  type t = {
    id : word * word;
    manufacturer : word * word;
    version : word
  }
end

(** User input data that is necessary for devices to function. *)
module Input = struct

  (** Input state. *)
  type t = {
    key_code : uint8 option;
  }

  (** The default (empty) input state. *)
  let none =
    { key_code = None; }
end

(** Common device interface. *)
type t = <
  (** Invoked the {! Engine} when the device receives an interrupt. *)
  on_interrupt : t Program.t IO.t;

  (** Invoked by the {! Engine} on every iteraton of the event loop which
      renders graphics and handles user input. *)
  on_interaction : Input.t -> Mem.t -> t IO.t;

  (** Invoked the {! Engine} on every instruction that is decoded from memory
      and executed.

      The device can indicate here that it has generated an interrupt for the
      processor. *)
  on_tick : (t * (Interrupt.t option)) IO.t;

  info : Info.t
>
