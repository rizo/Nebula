(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

type t = {
  memory : Mem.t;
  cpu : Cpu.t;
  ic : Interrupt_control.t;
  manifest : Manifest.t;
}

let default = {
  memory = Mem.empty;
  cpu = Cpu.empty;
  ic = Interrupt_control.empty;
  manifest = Manifest.empty;
}

let show t =
  let open Printf in

  let format name value =
    sprintf "%s: %s [%s]\n" name (Word.show value) (Word.show (Mem.read value t.memory))
  in

  let register name r =
    let value = Cpu.read_register r t.cpu in
    format name value
  in

  let special name s =
    let value = Cpu.read_special s t.cpu in
    format name value
  in

  let buffer = Buffer.create 20 in

  Buffer.add_string buffer (special "PC" Special.PC);
  Buffer.add_string buffer (special "SP" Special.SP);
  Buffer.add_string buffer (special "EX" Special.EX);
  Buffer.add_string buffer (special "IA" Special.IA);

  Buffer.add_string buffer "\n";

  Buffer.add_string buffer (register "A" Reg.A);
  Buffer.add_string buffer (register "B" Reg.B);
  Buffer.add_string buffer (register "C" Reg.B);
  Buffer.add_string buffer (register "X" Reg.X);
  Buffer.add_string buffer (register "Y" Reg.Y);
  Buffer.add_string buffer (register "Z" Reg.Z);
  Buffer.add_string buffer (register "I" Reg.I);
  Buffer.add_string buffer (register "J" Reg.J);

  Buffer.add_string buffer "\n";

  Buffer.add_string buffer ("Skip_next: " ^ string_of_bool (Cpu.get_flag Cpu.Flag.Skip_next t.cpu));

  Buffer.add_string buffer "\n";
  Buffer.contents buffer
