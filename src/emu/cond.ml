(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

module Cs = Computer_state

let sprintf = Printf.sprintf

type outcome =
  | Hit of string
  | Miss

type t = {
  descr : string;
  apply : Computer_state.t -> bool * t option
}

let check state t =
  let (hit, next) = t.apply state in

  match hit with
  | true -> (Hit t.descr, next)
  | false -> (Miss, next)

let break offset =
  let rec this = lazy {
    descr = sprintf "Hit break-point at 0x%04x." (Word.to_int offset);
    apply = fun state -> begin
        (Cpu.read_special Special.PC state.Cs.cpu = offset,
         Some (Lazy.force this))
      end
  }
  in
  Lazy.force this

let continue = {
  descr = "<continue>";
  apply = fun _ -> (false, None)
}

let rec step n =
  let descr =
    sprintf
      "Stepped over %s."
      (if n <= 1 then "1 instruction" else (string_of_int n) ^ " instructions")
  in

  let rec this = lazy {
    descr;
    apply = fun _ -> f n
  }
  and f n =
    if n <= 1 then (true, None)
    else (false, Some { (Lazy.force this) with apply = fun _ -> f (n - 1) })
  in
  Lazy.force this
