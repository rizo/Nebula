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

  (* val interrupt : word -> t Program.t *)

  val info : Info.t
end

module type Instance = sig
  module Device : S

  val this : Device.t
end
