open Prelude
open Functional

module Cells = Map.Make(Word)

type t = word Cells.t

let empty =
  Cells.empty

let default_value =
  word 0

let read w t =
  try
    Cells.find w t
  with
  | Not_found -> default_value

let write w o t =
  Cells.add w o t

let of_bytes bytes =
  let num_bytes = Bytes.length bytes in
  let num_words = num_bytes / 2 in
  let memory = ref empty in

  for index = 0 to (num_words - 1)  do
    let even_index = index * 2 in
    let higher_order = int_of_char (Bytes.get bytes even_index) in
    let lower_order = int_of_char (Bytes.get bytes (even_index + 1)) in
    let w = (higher_order lsl 8) lor lower_order |> Word.of_int in
    memory := !memory |> Cells.add (word index) w
  done;

  !memory

let of_file filename =
  IO.lift begin fun () ->
    try
      let channel = open_in_bin filename in
      let size = in_channel_length channel in
      let buffer = Bytes.create size in
      really_input channel buffer 0 size;
      close_in channel;

      Right (of_bytes buffer)
    with
    | Sys_error message -> Left (`Bad_memory_file message)
  end
