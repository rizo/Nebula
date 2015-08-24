(** Simulated hardware device. *)

open Functional
open Prelude

open Unsigned

module Info = struct
  type t = {
    id : word * word;
    manufacturer : word * word;
    version : word
  }
end

module Input = struct
  type t = {
    key_code : uint8 option;
  }

  let none =
    { key_code = None; }
end

module type S = sig
  type t

  val on_interrupt : t -> t Program.t IO.t

  val on_interaction : Input.t -> Mem.t -> t -> t IO.t

  val on_tick : t -> (t * (Interrupt.t option)) IO.t

  val info : Info.t
end

module type Instance = sig
  module Device : S

  val this : Device.t

  val index : device_index
end

let make_instance (type a) (module D : S with type t = a) this index =
  (module struct
    module Device = D

    let index = index

    let this = this
  end : Instance)
