type direct
type indirect
type offset
type stack

type (_, _) t =
  | I : Word.t -> (direct, Word.t) t
