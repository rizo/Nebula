(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

type word = Word.t

let word =
  Word.of_int

type device_index = word

type message = word
