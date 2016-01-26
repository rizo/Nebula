(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

module Cs = Computer_state

open Functional
open Functional.Prelude

module Repl = struct
  open IO.Monad

  module Command = struct
    type t =
      | Step of int
      | Continue
      | Show_state
      | Exit

    let parse = function
      | "step" -> Some (Step 1)
      | "continue" -> Some Continue
      | "state" -> Some Show_state
      | "exit" -> Some Exit
      | _ -> None
  end

  let show_prompt =
    IO.put_string ">>> "

  let show_reasons reasons =
    let combined = String.concat "\n" (List.map (fun r -> "- " ^ r) reasons) in
    IO.put_string combined >>= fun () ->
    IO.put_string "\n"

  let get_line =
    IO.catch (IO.lift read_line |> IO.Functor.map begin fun line ->
        if line = "" then None else Some line
      end)
      (fun _ -> IO.unit None)
end

module Int_map = Map.Make (struct
    type t = int
    let compare = compare
  end)

type t = {
  conditions : Cond.t Int_map.t;
  next_index : int;
  unconditional_hit : bool;
}

let empty = {
  conditions = Int_map.empty;
  next_index = 0;
  unconditional_hit = false;
}

let empty_with_unconditional_hit =
  { empty with unconditional_hit = true }

let add_condition cond t =
  let conditions = Int_map.add t.next_index cond t.conditions in
  { t with conditions; next_index = t.next_index + 1 }

let map_of_list ls =
  let map = ref Int_map.empty in
  List.iter (fun (k, v) -> map := Int_map.add k v !map) ls;
  !map

let evaluate_conditions cs t =
  let checked = Int_map.map (fun cond -> Cond.check cs cond) t.conditions in

  let next_conds =
    Int_map.map snd checked
    |> Int_map.bindings
    |> List.map (fun (index, cond_option) ->
        match cond_option with
        | Some cond -> Some (index, cond)
        | None -> None)
    |> list_of_options
    |> map_of_list
  in

  let reasons =
    Int_map.map fst checked
    |> Int_map.bindings
    |> filter_map begin fun (_, outcome) ->
      match outcome with
      | Cond.Hit descr -> Some descr
      | Cond.Miss -> None
    end
  in
  (reasons, { t with conditions = next_conds })

let rec main_loop c t =
  let open IO.Monad in

  Repl.show_prompt >>= fun () ->
  Repl.get_line >>= fun maybe_input ->

  match maybe_input with
  | None -> main_loop c t
  | Some input -> begin
      match Repl.Command.parse input with
      | Some (Repl.Command.Step n) -> begin
          IO.unit (add_condition (Cond.step n) t)
        end
      | Some Repl.Command.Exit -> IO.terminate 0
      | Some Repl.Command.Continue -> begin
          IO.unit (add_condition Cond.continue t)
        end
      | Some Repl.Command.Show_state -> begin
          IO.put_string (Cs.show c) >>= fun () -> main_loop c t
        end
      | None -> begin
          IO.put_string ("Invalid input: `" ^ input ^ "`\n") >>= fun () ->
          main_loop c t
        end
    end

let yield (c : Computer_state.t) (t : t) =
  let open IO.Monad in

  if t.unconditional_hit then
    main_loop c { t with unconditional_hit = false }
  else begin
    let (reasons, t) = evaluate_conditions c t in

    match reasons with
    | [] -> IO.unit t
    | reasons -> begin
        Repl.show_reasons reasons >>= fun () ->
        main_loop c t
      end
  end
