open Functional
open Functional.Prelude

module Inner = State.Make (Dcpu16_state)

module Error = struct
  type t =
    | Already_defined_label of string
end

module Self = Either_trans.Make (Error) (Inner.Monad_instance)

include Self

let run pc t =
  run t |> Inner.run (Dcpu16_state.beginning_at pc)

let assemble ?(at = Word.of_int 0) t =
  run at t |> begin function
    | (_, Left e) -> Left e
    | (s, Right _) -> begin
        Right Dcpu16_state.(s.encoded, labels_list s)
      end
  end

let emit ws =
  lift @@ Inner.modify begin fun { Dcpu16_state.pc; encoded; labels } ->
    Dcpu16_state.{
      pc = Word.(pc + of_int (List.length ws));
      encoded = List.append encoded ws;
      labels;
    }
  end

let label =
  lift @@ Inner.gets (fun s -> Value.(A (I s.Dcpu16_state.pc)))

let define name body =
  let open Monad in

  lift @@ Inner.get >>= fun s ->
  let here = s.Dcpu16_state.pc in

  (match Dcpu16_state.lookup_label_loc name s with
  | Some (Label_loc.Fixed _) -> error (Error.Already_defined_label name)
  | Some (Label_loc.Unresolved) | None -> begin
      lift @@ Inner.modify (Dcpu16_state.set_label_loc name (Label_loc.Fixed here))
    end) >>= fun () -> body

let loc name =
  let open Monad in

  lift Inner.get >>= fun s ->
  match Dcpu16_state.lookup_label_loc name s with
  | Some (Label_loc.Fixed w) -> unit (Value.I w)
  | Some (Label_loc.Unresolved) | None -> begin
      lift (Inner.modify (Dcpu16_state.set_label_loc name Label_loc.Unresolved)) >>= fun () ->
      unit (Value.U name)
    end
