(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Prelude

module Reg_map = Map.Make(Reg)

module Special_map = Map.Make(Special)

type t = {
  registers : word Reg_map.t;
  specials : word Special_map.t;
  flags : flags;
}

and flags = {
  skip_next : bool;
}

module Flag = struct
  type t =
    | Skip_next
end

let read_register r t =
  Reg_map.find r t.registers

let write_register r v t =
  { t with registers = Reg_map.add r v t.registers }

let read_special s t =
  Special_map.find s t.specials

let write_special s v t =
  { t with specials = Special_map.add s v t.specials }

let empty =
  let open Reg in
  let open Special in
  {
    registers =
      Reg_map.(empty
                    |> add A (word 0) |> add B (word 0) |> add C (word 0)
                    |> add X (word 0) |> add Y (word 0) |> add Z (word 0)
                    |> add I (word 0) |> add J (word 0));
    specials =
      Special_map.(empty
                   |> add EX (word 0)
                   |> add SP (word 0)
                   |> add PC (word 0)
                   |> add IA (word 0));
    flags = { skip_next = false; }
  }

let set_flag flag value t =
  { t with
    flags =
      match flag with
      | Flag.Skip_next -> { skip_next = value; }
  }

let get_flag flag t =
  match flag with
  | Flag.Skip_next -> t.flags.skip_next
