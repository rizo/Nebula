(** Simulated computer monitor.

    A {b cell} is a single character on the simulated monitor. Cells are not
    square. An {b atom} is a single simulated pixel. A cell consists of many
    atoms (get it?).

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Common

open Functional

open Unsigned

module P = Program

type state =
  | Stopped
  | Starting of Time_stamp.t (** Time at which monitor was started. *)
  | Running

type t = {
  window : Visual.Window.t;
  state : state;
  last_blink_time : Time_stamp.t;
  blink_visible : bool;
  border_color_index : uint8;
  video_memory_offset : word;
  font_memory_offset : word;
  palette_memory_offset : word;
}

(** Default character font. *)
let default_font = [|
  0b1011011110011110, 0b0011100010001110; (* NULL *)
  0b0111001000101100, 0b0111010111110100; (* SOH *)
  0b0001100110111011, 0b0111111110001111; (* STX *)
  0b1000010111111001, 0b1011000101011000; (* ETX *)
  0b0010010000101110, 0b0010010000000000; (* EOT *)
  0b0000100000101010, 0b0000100000000000; (* ENQ *)
  0b0000000000001000, 0b0000000000000000; (* ACK *)
  0b0000100000001000, 0b0000100000001000; (* BEL *)
  0b0000000011111111, 0b0000000000000000; (* BS *)
  0b0000000011111000, 0b0000100000001000; (* TAB *)
  0b0000100011111000, 0b0000000000000000; (* LF *)
  0b0000100000001111, 0b0000000000000000; (* VT *)
  0b0000000000001111, 0b0000100000001000; (* FF *)
  0b0000000011111111, 0b0000100000001000; (* CR *)
  0b0000100011111000, 0b0000100000001000; (* SO *)
  0b0000100011111111, 0b0000000000000000; (* SI *)
  0b0000100000001111, 0b0000100000001000; (* DLE *)
  0b0000100011111111, 0b0000100000001000; (* DC1 *)
  0b0110011000110011, 0b1001100111001100; (* DC2 *)
  0b1001100100110011, 0b0110011011001100; (* DC3 *)
  0b1111111011111000, 0b1110000010000000; (* DC4 *)
  0b0111111100011111, 0b0000011100000001; (* NAK *)
  0b0000000100000111, 0b0001111101111111; (* SYN *)
  0b1000000011100000, 0b1111100011111110; (* ETB *)
  0b0101010100000000, 0b1010101000000000; (* CAN *)
  0b0101010110101010, 0b0101010110101010; (* EM *)
  0b1111111110101010, 0b1111111101010101; (* SUB *)
  0b0000111100001111, 0b0000111100001111; (* ESC *)
  0b1111000011110000, 0b1111000011110000; (* FS *)
  0b0000000000000000, 0b1111111111111111; (* GS *)
  0b1111111111111111, 0b0000000000000000; (* RS *)
  0xffff, 0xffff;                         (* US *)
  0, 0;                                   (* Space *)
  0b0000000010111111, 0b0000000000000000; (* ! *)
  0b0000001100000000, 0b0000001100000000; (* double-quote *)
  0b0011111000010100, 0b0011111000000000; (* # *)
  0b0100110011010110, 0b0110010000000000; (* $ *)
  0b1100001000111000, 0b1000011000000000; (* % *)
  0b0110110001010010, 0b1110110010100000; (* & *)
  0b0000000000000010, 0b0000000100000000; (* ' *)
  0b0011110001000010, 0b1000000100000000; (* ( *)
  0b1000000101000010, 0b0011110000000000; (* ) *)
  0b0000101000000100, 0b0000101000000000; (* * *)
  0b0000100000011100, 0b0000100000000000; (* + *)
  0b0000000010000000, 0b0100000000000000; (* , *)
  0b0000100000001000, 0b0000100000000000; (* - *)
  0b0000000010000000, 0b0000000000000000; (* . *)
  0b1100000000111000, 0b0000011000000000; (* / *)
  0b0111110010010010, 0b0111110000000000; (* 0 *)
  0b1000001011111110, 0b1000000000000000; (* 1 *)
  0b1100010010100010, 0b1001110000000000; (* 2 *)
  0b1000001010010010, 0b0110110000000000; (* 3 *)
  0b0001111000010000, 0b1111111000000000; (* 4 *)
  0b1001111010010010, 0b0110001000000000; (* 5 *)
  0b0111110010010010, 0b0110010000000000; (* 6 *)
  0b1100001000110010, 0b0000111000000000; (* 7 *)
  0b0110110010010010, 0b0110110000000000; (* 8 *)
  0b0100110010010010, 0b0111110000000000; (* 9 *)
  0b0000000001001000, 0b0000000000000000; (* : *)
  0b0000000010000000, 0b0100100000000000; (* ; *)
  0b0001000000101000, 0b0100010000000000; (* < *)
  0b0010010000100100, 0b0010010000000000; (* = *)
  0b0100010000101000, 0b0001000000000000; (* > *)
  0b0000001010110001, 0b0000111000000000; (* ? *)
  0b0111110010110010, 0b1011110000000000; (* @ *)
  0b1111110000010010, 0b1111110000000000; (* A *)
  0b1111111010010010, 0b0110110000000000; (* B *)
  0b0111110010000010, 0b0100010000000000; (* C *)
  0b1111111010000010, 0b0111110000000000; (* D *)
  0b1111111010010010, 0b1001001000000000; (* E *)
  0b1111111000010010, 0b0001001000000000; (* F *)
  0b0111110010000010, 0b1110010000000000; (* G *)
  0b1111111000010000, 0b1111111000000000; (* H *)
  0b1000001011111110, 0b1000001000000000; (* I *)
  0b0100001010000010, 0b1111111000000000; (* J *)
  0b1111111000010000, 0b1110111000000000; (* K *)
  0b1111111010000000, 0b1000000000000000; (* L *)
  0b1111111000001100, 0b1111111000000000; (* M *)
  0b1111111000000010, 0b1111110000000000; (* N *)
  0b0111110010000010, 0b0111110000000000; (* O *)
  0b1111111000010010, 0b0000110000000000; (* P *)
  0b0111110011000010, 0b1111110000000000; (* Q *)
  0b1111111000010010, 0b1110110000000000; (* R *)
  0b1000110010010010, 0b0110001000000000; (* S *)
  0b0000001011111110, 0b0000001000000000; (* T *)
  0b0111111010000000, 0b0111111000000000; (* U *)
  0b0011111011000000, 0b0011111000000000; (* V *)
  0b1111111001100000, 0b1111111000000000; (* W *)
  0b1110111000010000, 0b1110111000000000; (* X *)
  0b0000111011110000, 0b0000111000000000; (* Y *)
  0b1110001010010010, 0b1000111000000000; (* Z *)
  0b1111111010000010, 0b0000000000000000; (* [ *)
  0b0000011000111000, 0b1100000000000000; (* backslash *)
  0b0000000010000010, 0b1111111000000000; (* ] *)
  0b0000010000000010, 0b0000010000000000; (* ^ *)
  0b1000000010000000, 0b1000000000000000; (* _ *)
  0b0000001000000100, 0b0000000000000000; (* ` *)
  0b0100100010101000, 0b1111100000000000; (* a *)
  0b1111111010001000, 0b0111000000000000; (* b *)
  0b0111000010001000, 0b0101000000000000; (* c *)
  0b0111000010001000, 0b1111111000000000; (* d *)
  0b0111000010101000, 0b1011000000000000; (* e *)
  0b0001000011111100, 0b0001001000000000; (* f *)
  0b1001000010101000, 0b0111100000000000; (* g *)
  0b1111111000001000, 0b1111000000000000; (* h *)
  0b1000100011111010, 0b1000000000000000; (* i *)
  0b0100000010000000, 0b0111101000000000; (* j *)
  0b1111111000100000, 0b1101100000000000; (* k *)
  0b1000001011111110, 0b1000000000000000; (* l *)
  0b1111100000110000, 0b1111100000000000; (* m *)
  0b1111100000001000, 0b1111000000000000; (* n *)
  0b0111000010001000, 0b0111000000000000; (* o *)
  0b1111100000101000, 0b0001000000000000; (* p *)
  0b0001000000101000, 0b1111100000000000; (* q *)
  0b1111100000001000, 0b0001000000000000; (* r *)
  0b1001000010101000, 0b0100100000000000; (* s *)
  0b0000100011111100, 0b1000100000000000; (* t *)
  0b0111100010000000, 0b1111100000000000; (* u *)
  0b0011100011000000, 0b0011100000000000; (* v *)
  0b1111100001100000, 0b1111100000000000; (* w *)
  0b1101100000100000, 0b1101100000000000; (* x *)
  0b1001100010100000, 0b0111100000000000; (* y *)
  0b1100100010101000, 0b1001100000000000; (* z *)
  0b0001000001101100, 0b1000001000000000; (* { *)
  0b0000000011101110, 0b0000000000000000; (* | *)
  0b1000001001101100, 0b0001000000000000; (* } *)
  0b0000010000000010, 0b0000010000000010; (* ~ *)
  0b0000001000000101, 0b0000001000000000; (* DEL *)
|]
  |> Array.map (fun (a, b) -> word a, word b)

(** Width of the simulated monitor window in real pixels. *)
let width =
  640

(** Height of the simulated monitor window in real pixels. *)
let height =
  480

let cells_per_monitor_width =
  32

let cells_per_monitor_height =
  12

(** Width of a simulated cell in real pixels. *)
let cell_width =
  width / cells_per_monitor_width

(** Height of a simulated cell in real pixels. *)
let cell_height =
  height / cells_per_monitor_height

(** Size of a square simulated atom in real pixels. *)
let atom_size =
  cell_width / 4

let atoms_per_cell_width =
  cell_width / atom_size

let atoms_per_cell_height =
  cell_height / atom_size

(** Width of the vertical border in real pixels. *)
let border_width =
  2 * atom_size

(** Height of the horizontal border in real pixels. *)
let border_height =
  2 * atom_size

(** Width of the monitor window in real pixels, including the width of the border. *)
let total_width =
  width + (2 * border_width)

(** Height of the monitor window in real pixels, including the height of the border. *)
let total_height =
  height + (2 * border_height)

(** Duration of time between blinking characters. *)
let blink_period =
  Duration.of_nanoseconds 1000000000L

(** Duration of time that the start-up screen appears. *)
let start_up_duration =
  Duration.of_nanoseconds 1300000000L

(** Default color palette.

    Colors are stored in an encoded form, which can be decoded via {! decode_color}. *)
let default_palette =
  [| 0x000; 0x00a; 0x0a0; 0x0aa; 0xa00; 0xa0a; 0xa50; 0xaaa;
     0x555; 0x55f; 0x5f5; 0x5ff; 0xf55; 0xf5f; 0xff5; 0xfff; |]
  |> Array.map Word.of_int

let decode_color color_code =
  (* Since only 4 bits are provided for each channel, we shift everything to
     the left by 4 bits so that the bits that _are_ provided are interpreted as
     high-order bits. *)
  let r = (color_code land 0x0f00) lsr 4 |> UInt8.of_int in
  let g = (color_code land 0x00f0) |> UInt8.of_int in
  let b = (color_code land 0x000f) lsl 4 |> UInt8.of_int in
  Visual.Color.make ~r ~g ~b

let color_of_index index memory t =
  let color_code_word =
    if t.palette_memory_offset <> word 0 then
      Mem.read Word.(t.palette_memory_offset + (of_int (UInt8.to_int index))) memory
    else
      default_palette.(UInt8.to_int index)
  in
  let color_code = Word.to_int color_code_word in
  decode_color color_code

let draw_border memory t =
  let window = t.window in
  let border_color = color_of_index t.border_color_index memory t in

  IO.Monad.sequence_unit Visual.[
      set_color window border_color;

      (* Left. *)
      rectangle
        ~origin:(0, border_height) ~width:border_width ~height window;

      (* Right. *)
      rectangle
        ~origin:(total_width - border_width, border_width) ~width:border_width ~height window;

      (* Top. *)
      rectangle
        ~origin:(0, 0) ~width:total_width ~height:border_height window;

      (* Bottom. *)
      rectangle
        ~origin:(0, total_height - border_height) ~width:total_width ~height:border_height window;
    ]

let draw_atom window ~x_atom ~y_atom =
  let x = border_width + (x_atom * atom_size) in
  let y = border_height + (y_atom * atom_size) in

  Visual.rectangle
    ~origin:(x, y)
    ~width:atom_size
    ~height:atom_size
    window

let draw_cell ~x_cell ~y_cell ~character_index ~fg_color_index ~bg_color_index ~blink memory t =
  let open IO.Monad in

  IO.lift begin fun () ->
    let fg_color = color_of_index fg_color_index memory t in
    let bg_color = color_of_index bg_color_index memory t in

    let base_x_atom_offset = x_cell * atoms_per_cell_width in
    let base_y_atom_offset = y_cell * atoms_per_cell_height in

    let draw_column column_data x_atom =
      for y_atom = 0 to (atoms_per_cell_height - 1) do
        let is_fg =
          Word.((column_data land (word 1 lsl y_atom)) |> to_bool) &&
          ((blink && t.blink_visible) || (not blink))
        in

        IO.unsafe_perform begin
          Visual.set_color t.window (if is_fg then fg_color else bg_color) >>= fun () ->
          draw_atom
            ~x_atom:(base_x_atom_offset + x_atom)
            ~y_atom:(base_y_atom_offset + y_atom)
            t.window
        end
      done
    in

    let ch =
      if t.font_memory_offset <> word 0 then
        let i = UInt8.to_int character_index |> Word.of_int in
        (Mem.read Word.(t.font_memory_offset + (word 2 * i)) memory,
         Mem.read Word.((t.font_memory_offset + (word 2 * i)) + word 1) memory)
      else
        default_font.(UInt8.to_int character_index)
    in

    draw_column Word.((fst ch land word 0xff00) lsr 8) 0;
    draw_column Word.(fst ch land word 0x00ff) 1;
    draw_column Word.((snd ch land word 0xff00) lsr 8) 2;
    draw_column Word.(snd ch land word 0x00ff) 3;
  end

let draw_from_memory memory t =
  IO.lift begin fun () ->
    let draw ~x_cell ~y_cell =
      let memory_offset =
        Word.((word y_cell * word cells_per_monitor_width) + (word x_cell) + t.video_memory_offset)
      in

      let w = Mem.read memory_offset memory in
      let blink = Word.(w land word 0x0080 <> word 0) in
      let character_index = Word.(w land word 0x007f |> to_int) |> UInt8.of_int in
      let fg_color_index = Word.((w land word 0xf000) lsr 12 |> to_int) |> UInt8.of_int in
      let bg_color_index = Word.((w land word 0x0f00) lsr 8 |> to_int) |> UInt8.of_int in

      IO.unsafe_perform begin
        draw_cell ~x_cell ~y_cell ~character_index ~fg_color_index ~bg_color_index ~blink
          memory
          t
      end
    in

    for x_cell = 0 to (cells_per_monitor_width - 1) do
      for y_cell = 0 to (cells_per_monitor_height -1) do
        draw ~x_cell ~y_cell
      done
    done
  end

(** Memory loaded with the default font, for demonstration purposes.

    The font is stored starting at offset [0x4000]. *)
let memory_with_default_font =
  let rec loop memory offset =
    if Word.to_int offset = Array.length default_font then memory
    else
      let memory =
        Mem.write
          Word.(offset + word 0x4000)
          Word.((word 0xf lsl 12) lor word 0x80 lor offset)
          memory
      in
      loop memory Word.(offset + word 1)
  in
  loop Mem.empty (word 0)

let draw_default_font t =
  draw_from_memory memory_with_default_font t

let draw_startup_screen window =
  let startup_color = default_palette.(1) |> Word.to_int |> decode_color in
  IO.Monad.(Visual.set_color window startup_color >>= fun () -> Visual.clear window)

let render memory t =
  let open IO.Monad in

  let window = t.window in

  IO.Monad.sequence_unit Visual.[
    set_color window Color.black;
    clear window;

    (match t.state with
     | Stopped -> IO.unit ()
     | Starting _ -> draw_startup_screen window >>= fun () -> draw_border memory t
     | Running -> draw_from_memory memory t >>= fun () -> draw_border memory t);

    render window
  ]

let make window =
  let open IO.Functor in
  Precision_clock.get_time |> map (fun now ->
  { window;
    state = Stopped;
    last_blink_time = now;
    blink_visible = true;
    border_color_index = UInt8.of_int 1;
    video_memory_offset = word 0;
    font_memory_offset = word 0;
    palette_memory_offset = word 0;
  })

let on_tick t =
  let open IO.Monad in

  Precision_clock.get_time >>= fun now ->
  let elapsed_since_blink = Duration.of_nanoseconds Time_stamp.(now - t.last_blink_time) in

  let t =
    if elapsed_since_blink >= blink_period then
      { t with last_blink_time = now; blink_visible = not t.blink_visible }
    else t
  in

  let t =
    match t.state with
    | Starting start_time -> begin
        let elapsed_since_starting = Duration.of_nanoseconds Time_stamp.(now - start_time) in
        if elapsed_since_starting >= start_up_duration then { t with state = Running }
        else t
      end
    | Stopped | Running -> t
  in
  IO.unit (t, None)

let on_interrupt t =
  let open IO.Monad in

  Precision_clock.get_time >>= fun now ->

  IO.unit begin
    let open Program.Functor in
    let open Program.Monad in

    P.read_register Reg.B >>= fun b ->
    P.read_register Reg.A |> map Word.to_int >>= function
    | 0 -> begin (* MEM_MAP_SCREEN *)
        P.Return {
          t with
          video_memory_offset = b;
          state =
            if (b <> word 0) then
              if t.state = Stopped then Starting now else Running
            else Stopped
        }
      end
    | 1 -> begin (* MEM_MAP_FONT *)
        P.Return { t with font_memory_offset = b }
      end
    | 2 -> begin (* MEM_MAP_PALETTE *)
        P.Return { t with palette_memory_offset = b }
      end
    | 3 -> begin (* SET_BORDER_COLOR *)
        P.Return {
          t with
          border_color_index = UInt8.of_int (Word.(b land word 0xf |> to_int))
        }
      end
    | 4 -> begin (* MEM_DUMP_FONT *)
        let rec loop offset =
          if offset >= Array.length default_font then P.Return t
          else
            let (w1, w2) = default_font.(offset) in
            let v = Word.(b + (word 2 * word offset)) in
            P.write_memory v w1 >>= fun () ->
            P.write_memory Word.(v + word 1) w2 >>= fun () ->
            loop (offset + 1)
        in
        loop 0
      end
    | 5 -> begin (* MEM_DUMP_PALETTE *)
        let rec loop offset =
          if offset >= Array.length default_palette then P.Return t
          else
            P.write_memory Word.(b + word offset) default_palette.(offset) >>= fun () ->
            loop (offset + 1)
        in
        loop 0
      end
    | _ -> P.Return t
  end

let on_interaction device_input memory t =
  IO.Monad.(render memory t >>= fun () -> IO.unit t)

let info =
  Device.Info.{
    id = (word 0x7349, word 0xf615);
    manufacturer = (word 0x1c6c, word 0x8b36);
    version = word 0x1802
  }
