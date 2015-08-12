open Prelude

module Info = struct
  type t = {
    id : word * word;
    manufacturer : word * word;
    version : word
  }
end

module type S = sig
  type t

  val on_interrupt : message -> t -> t Program.t

  val on_interaction : t -> t IO.t

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
