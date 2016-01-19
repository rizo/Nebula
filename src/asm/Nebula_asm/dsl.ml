include Value

include Dcpu16.Functor
include Dcpu16.Monad
include Dcpu16.Applicative

let w = Word.of_int

let define = Dcpu16.define
let label = Dcpu16.label
let assemble = Dcpu16.assemble
let assemble_and_link = Dcpu16.assemble_and_link

let push = Push
let pop = Pop
let peek = Peek
let pick i = Pick (w i)
let sp = Sp
let pc = Pc
let ex = Ex

let ra = R Reg.A
let rb = R Reg.B
let rc = R Reg.C
let rx = R Reg.X
let ry = R Reg.Y
let rz = R Reg.Z
let ri = R Reg.I
let rj = R Reg.J

let imm i = I (w i)
let ind : type a. (direct, a) Value.t -> (indirect, a) Value.t = fun i -> A i
let at a = A a
let off i r = D (r, i)

let unary special_code = fun a -> Dcpu16.assemble_inst @@ Inst.(Unary (special_code, a))
let binary code = fun b a -> Dcpu16.assemble_inst @@ Inst.(Binary (code, b, a))

let set b a = binary Inst.Code.Set b a
let add b a = binary Inst.Code.Add b a
let sub b a = binary Inst.Code.Sub b a
let mul b a = binary Inst.Code.Mul b a
let mli b a = binary Inst.Code.Mli b a
let div b a = binary Inst.Code.Div b a
let dvi b a = binary Inst.Code.Dvi b a
let mad b a = binary Inst.Code.Mod b a
let mdi b a = binary Inst.Code.Mdi b a
let und b a = binary Inst.Code.And b a
let bor b a = binary Inst.Code.Bor b a
let xor b a = binary Inst.Code.Xor b a
let shr b a = binary Inst.Code.Shr b a
let sra b a = binary Inst.Code.Asr b a
let shl b a = binary Inst.Code.Shl b a
let ifb b a = binary Inst.Code.Ifb b a
let ifc b a = binary Inst.Code.Ifc b a
let ife b a = binary Inst.Code.Ife b a
let ifn b a = binary Inst.Code.Ifn b a
let ifg b a = binary Inst.Code.Ifg b a
let ifa b a = binary Inst.Code.Ifa b a
let ifl b a = binary Inst.Code.Ifl b a
let ifu b a = binary Inst.Code.Ifu b a
let adx b a = binary Inst.Code.Adx b a
let sbx b a = binary Inst.Code.Sbx b a
let sti b a = binary Inst.Code.Sti b a
let std b a = binary Inst.Code.Std b a

let jsr a = unary Inst.Special_code.Jsr a
let int a = unary Inst.Special_code.Int a
let iag a = unary Inst.Special_code.Iag a
let ias a = unary Inst.Special_code.Ias a
let rfi a = unary Inst.Special_code.Rfi a
let iaq a = unary Inst.Special_code.Iaq a
let hwn a = unary Inst.Special_code.Hwn a
let hwq a = unary Inst.Special_code.Hwq a
let hwi a = unary Inst.Special_code.Hwi a

let dat xs = Dcpu16.emit (List.map (fun x -> Assembled.Constant (w x)) xs)

let asciip s =
  let chars = ref [] in
  String.iter (fun ch -> chars := int_of_char ch :: !chars) s;
  dat (String.length s :: (List.rev !chars))

let ( >> ) x y =
  x >>= fun () -> y

let void t =
  t >>= fun _ -> Dcpu16.unit ()

exception Assembly_error of Dcpu16.Error.t

let format_exception exn =
  let sprintf = Printf.sprintf in

  match exn with
  | Assembly_error ae -> begin
      match ae with
      | Dcpu16.Error.Undefined_labels labels -> begin
          sprintf "Error: the following labels are never defined: %s."
            (String.concat "," (List.map (fun label -> "`" ^ label ^ "`") labels))
        end
      | Dcpu16.Error.Already_defined_label label -> begin
          sprintf "Error: the label `%s` is defined multiple times." label
        end
    end
  | _ -> Printexc.to_string exn

let assembly_main ~file_name t =
  let open Functional in
  let open Functional.Prelude in

  try
    begin match Dcpu16.assemble_and_link t with
      | Left error -> IO.throw (Assembly_error error)
      | Right encoded -> Dcpu16.write_file ~file_name encoded
    end
    |> IO.main
  with
  | exn -> prerr_endline (format_exception exn)
