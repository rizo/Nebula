(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

open Functional

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

type t = <
  on_interrupt : t Program.t IO.t;
  on_interaction : Input.t -> Mem.t -> t IO.t;
  on_tick : (t * (Interrupt.t option)) IO.t;
  info : Info.t
>
