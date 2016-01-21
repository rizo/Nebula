(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

type direct
type indirect
type offset
type stack
type special

module Reg = struct
  type t =
    | A | B | C
    | X | Y | Z
    | I | J
end

type (_, _) t =
  | I : Word.t -> (direct, Word.t) t
  | R : Reg.t -> (direct, Reg.t) t
  | L : string -> (direct, Word.t) t
  | A : (direct, 'a) t -> (indirect, 'a) t
  | D : (direct, Reg.t) t * (direct, Word.t) t -> (offset, Reg.t) t
  | Push : (stack, unit) t
  | Pop : (stack, unit) t
  | Peek : (special, unit) t
  | Pick : Word.t -> (special, Word.t) t
  | Sp : (special, unit) t
  | Pc : (special, unit) t
  | Ex : (special, unit) t

type address = (indirect, Word.t) t
