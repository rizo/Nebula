(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

type word = Word.t

let word =
  Word.of_int

type device_index = word

type message = word

module Time_stamp = struct
  type t = int64

  let ( - ) a b =
    Int64.sub a b
end
