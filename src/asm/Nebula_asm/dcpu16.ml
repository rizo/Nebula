(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

module S = Dcpu16_state.String_map

open Functional
open Functional.Prelude

module Inner = State.Make (Dcpu16_state)

module Error = struct
  type t =
    | Already_defined_label of string
    | Undefined_labels of string list
end

module Self = Either_trans.Make (Error) (Inner.Monad_instance)

include Self

let run ?(labels = S.empty) pc t =
  run t |> Inner.run Dcpu16_state.{ (beginning_at pc) with labels }

module Block = struct
  type t = {
    encoded : Assembled.t list;
    labels : Dcpu16_state.label_map;
  }

  let unresolved_labels t =
    t.labels
    |> S.bindings
    |> List.map begin fun (name, loc) ->
      match loc with
      | Label_loc.Unresolved -> Some name
      | _ -> None
    end
    |> list_of_options

  let is_dependent t =
    List.exists (function Assembled.Dependent _ -> true | _ -> false) t.encoded

  let merge_labels t1 t2 =
    let link_ctx = Label_loc.Context.Link in

    begin
      S.merge
        begin fun name e1 e2 ->
          match (e1, e2) with
          | (None, None) -> None
          | (None, Some (Label_loc.Fixed (_, w)))
          | (Some (Label_loc.Fixed (_, w)), None) -> Some (Label_loc.Fixed (link_ctx, w))
          | (Some Label_loc.Unresolved, Some (Label_loc.Fixed (_, w)))
          | (Some (Label_loc.Fixed (_, w)), Some Label_loc.Unresolved) -> begin
              Some (Label_loc.Fixed (link_ctx, w))
            end
          | (None, Some loc) | (Some loc, None) -> Some loc
          | (Some Label_loc.Unresolved, Some Label_loc.Unresolved) -> Some Label_loc.Unresolved
          | (Some (Label_loc.Fixed _), Some (Label_loc.Fixed (_, f2))) -> begin
              Some (Label_loc.Fixed (link_ctx, f2))
            end
        end
        t1.labels
        t2.labels
    end
    |> fun ms -> Right ms
end

let assemble ?(at = Word.of_int 0) ?(labels = S.empty) t =
  run ~labels at t |> begin function
    | (_, Left e) -> Left e
    | (s, Right _) -> begin
        Right Block.{ encoded = s.Dcpu16_state.encoded; labels = s.Dcpu16_state.labels }
      end
  end

let assemble_and_link ?(at = Word.of_int 0) t =
  let do_pass ~labels =
    let open Monad in
    of_either (assemble ~at ~labels t) >>= fun b ->
    let labels =
      b.Block.labels
      |> S.map begin function
        | Label_loc.Fixed (_, f) -> Label_loc.Fixed (Label_loc.Context.Link, f)
        | loc -> loc
      end
    in
    unit Block.{ b with labels }
  in

  let rec go iter_block =
    let open Monad in

    if Block.is_dependent iter_block then begin
      match Block.unresolved_labels iter_block with
      | [] -> begin
          do_pass ~labels:(iter_block.Block.labels) >>= fun block ->
          of_either (Block.merge_labels iter_block block) >>= fun labels ->
          go { block with Block.labels = labels }
        end
      | unresolved -> error (Error.Undefined_labels unresolved)
    end
    else
      do_pass ~labels:(iter_block.Block.labels) >>= fun block ->
      block.Block.encoded
      |> List.map begin function
        | Assembled.Constant w -> w
        | _ -> failwith "Internal failure: Dependent expression!"
      end
      |> unit
  in

  run at Monad.(do_pass ~labels:(S.empty) >>= go)
  |> snd

let emit ws =
  lift @@ Inner.modify begin fun { Dcpu16_state.pc; encoded; labels } ->
    Dcpu16_state.{
      pc = Word.(pc + of_int (List.length ws));
      encoded = List.append encoded ws;
      labels;
    }
  end

let label =
  lift @@ Inner.gets (fun s -> Word.to_int s.Dcpu16_state.pc)

let define name body =
  let open Monad in

  lift @@ Inner.get >>= fun s ->
  let here = s.Dcpu16_state.pc in

  (match Dcpu16_state.lookup_label_loc name s with
   | Some (Label_loc.Fixed (Label_loc.Context.Assemble, _)) -> begin
       error (Error.Already_defined_label name)
     end
   | Some (Label_loc.Fixed (Label_loc.Context.Link, _))
   | Some (Label_loc.Unresolved) | None -> begin
       lift begin
         Inner.modify
           (Dcpu16_state.set_label_loc
              name
              (Label_loc.Fixed (Label_loc.Context.Assemble, here)))
       end
     end)
  >>= fun () -> body

let try_resolve_loc name =
  let open Monad in

  lift Inner.get >>= fun s ->
  match Dcpu16_state.lookup_label_loc name s with
  | Some (Label_loc.Fixed (_, w)) -> unit (Some w)
  | Some (Label_loc.Unresolved) | None -> begin
      lift (Inner.modify (Dcpu16_state.set_label_loc name Label_loc.Unresolved)) >>= fun () ->
      unit None
    end

let assemble_value : type a b. (a, b) Value.t -> (Assembled.t * Assembled.t option) t = fun v ->
  let module A = Assembled in
  let open Value in
  let open Monad in

  let reg_encoding r =
    begin match r with
    | Reg.A -> 0
    | Reg.B -> 1
    | Reg.C -> 2
    | Reg.X -> 3
    | Reg.Y -> 4
    | Reg.Z -> 5
    | Reg.I -> 6
    | Reg.J -> 7
    end
    |> Word.of_int
  in

  let short w = unit (A.Constant w, None) in
  let long w v = unit (A.Constant w, Some (A.Constant v)) in
  let long_dep name = unit (A.Dependent name, Some (A.Dependent name)) in

  let literal w =
    if Word.((w = of_int 0xffff) || ((w >= of_int 0) && (w <= of_int 0x1e))) then
      short Word.(w + of_int 0x21)
    else
      long (Word.of_int 0x1f) w
  in

  let indirect w = long (Word.of_int 0x1e) w in

  let resolve_loc_or_dep f name =
    try_resolve_loc name >>= function
    | Some w -> f w
    | None -> long_dep name
  in

  match v with
  | I w -> literal w
  | R r -> short (reg_encoding r)
  | L name -> resolve_loc_or_dep literal name
  | A (I w) -> indirect w
  | A (L name) -> resolve_loc_or_dep indirect name
  | A (R r) -> short Word.(reg_encoding r + of_int 8)
  | D (R r, I w) -> long Word.(reg_encoding r + of_int 0x10) w
  | D (R r, L name) -> begin
      resolve_loc_or_dep (fun w -> long Word.(reg_encoding r + of_int 0x10) w) name
    end
  | Push -> short (Word.of_int 0x18)
  | Pop -> short (Word.of_int 0x18)
  | Peek -> short (Word.of_int 0x19)
  | Pick w -> long (Word.of_int 0x1a) w
  | Sp -> short (Word.of_int 0x1b)
  | Pc -> short (Word.of_int 0x1c)
  | Ex -> short (Word.of_int 0x1d)
  | _ -> failwith "Internal failure: unmatched value!"

let assemble_inst inst =
  let module V = Value in

  let open Monad in

  let code_encoding c =
    let module I = Inst.Code in

    begin match c with
      | I.Set -> 0x01
      | I.Add -> 0x02
      | I.Sub -> 0x03
      | I.Mul -> 0x04
      | I.Mli -> 0x05
      | I.Div -> 0x06
      | I.Dvi -> 0x07
      | I.Mod -> 0x08
      | I.Mdi -> 0x09
      | I.And -> 0x0a
      | I.Bor -> 0x0b
      | I.Xor -> 0x0c
      | I.Shr -> 0x0d
      | I.Asr -> 0x0e
      | I.Shl -> 0x0f
      | I.Ifb -> 0x10
      | I.Ifc -> 0x11
      | I.Ife -> 0x12
      | I.Ifn -> 0x13
      | I.Ifg -> 0x14
      | I.Ifa -> 0x15
      | I.Ifl -> 0x16
      | I.Ifu -> 0x17
      | I.Adx -> 0x1a
      | I.Sbx -> 0x1b
      | I.Sti -> 0x1e
      | I.Std -> 0x1f
    end
    |> Word.of_int
  in

  let special_code_encoding s =
    let module I = Inst.Special_code in

    begin match s with
      | I.Jsr -> 0x01
      | I.Int -> 0x08
      | I.Iag -> 0x09
      | I.Ias -> 0x0a
      | I.Rfi -> 0x0b
      | I.Iaq -> 0x0c
      | I.Hwn -> 0x10
      | I.Hwq -> 0x11
      | I.Hwi -> 0x12
    end
    |> Word.of_int
  in

  match inst with
  | Inst.Unary (s, a) -> begin
      assemble_value a >>= fun (asm, extra) ->
      match asm with
      | Assembled.Dependent _ -> emit (list_of_options [Some asm; extra])
      | Assembled.Constant w -> begin
          let result =
            Word.(of_int 0 lor ((special_code_encoding s land of_int 0x1f) lsl 5))
          in

          let result = Word.(result lor ((w land of_int 0x3f) lsl 10)) in
          emit (Assembled.Constant result :: list_of_options [extra])
        end
    end
  | Inst.Binary (c, b, a) -> begin
      assemble_value b >>= fun (asmB, extraB) ->
      assemble_value a >>= fun (asmA, extraA) ->

      match (asmA, asmB) with
      | (Assembled.Dependent name, _) | (_, Assembled.Dependent name) -> begin
          emit (list_of_options [Some (Assembled.Dependent name); extraA; extraB])
        end
      | (Assembled.Constant wa, Assembled.Constant wb) -> begin
          let result = Word.(code_encoding c land of_int 0x1f) in
          let result = Word.(result lor ((wb land of_int 0x1f) lsl 5)) in
          let result = Word.(result lor ((wa land of_int 0x3f) lsl 10)) in
          emit (Assembled.Constant result :: list_of_options [extraA; extraB])
        end
    end

let write_file ~file_name encoded =
  let open IO.Monad in

  let size = 2 * List.length encoded in

  IO.lift (fun () -> open_out_bin file_name) >>= fun channel ->
  IO_ref.make (Bytes.make size (char_of_int 0)) >>= fun buffer_ref ->

  encoded
  |> List.mapi begin fun list_index w ->
    let i = Word.to_int w in
    let lower_order = char_of_int (i land 0xff) in
    let higher_order = char_of_int ((i land 0xff00) lsr 8) in
    let index = 2 * list_index in

    IO_ref.get buffer_ref >>= fun buffer ->
    Bytes.set buffer index higher_order;
    Bytes.set buffer (index + 1) lower_order;
    IO.unit ()
  end
  |> sequence_unit >>= fun () ->
  IO_ref.get buffer_ref >>= fun buffer ->
  IO.lift (fun () -> output_bytes channel buffer) >>= fun () ->
  IO.lift (fun () -> close_out channel)
