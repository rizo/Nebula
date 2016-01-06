type direct
type indirect
type offset
type stack
type special

type (_, _) t =
  | I : Word.t -> (direct, Word.t) t
  | R : Register.t -> (direct, Register.t) t
  | A : (direct, 'a) t -> (indirect, 'a) t
  | D : (direct, Register.t) t * Word.t -> (offset, Register.t) t
  | Push : (stack, unit) t
  | Pop : (stack, unit) t
  | Peek : (special, unit) t
  | Pick : Word.t -> (special, Word.t) t
  | Sp : (special, unit) t
  | Pc : (special, unit) t
  | Ex : (special, unit) t
