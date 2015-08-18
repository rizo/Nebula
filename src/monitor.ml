(** Simulated computer monitor.

    A {b cell} is a single character on the simulated monitor. Cells are not
    square. An {b atom} is a single simulated pixel. A cell consists of many
    atoms (get it?). *)

open Prelude

open Unsigned

type state =
  | Stopped
  | Starting of int
  | Running

type t = {
  window : Visual.Window.t;
  state : state;
  last_blink_time : int;
  blink_visible : bool;
  border_color_index : uint8;
  video_memory_offset : word;
  font_memory_offset : word;
  palette_memory_offset : word;
}

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

(** Duration of time between blinking characters, in nanoseconds. *)
let blink_period =
  1000000000

(** Duration of time that the start-up screen appears, in nanoseconds. *)
let start_up_duration =
  1300000000

(** Default color palette.

    Colors are stored in an encoded form, which can be decoded via {! decode_color}. *)
let default_palette =
  [| 0x000; 0x00a; 0x0a0; 0x0aa; 0xa00; 0xa0a; 0xa50; 0xaaa;
     0x555; 0x55f; 0x5f5; 0x5ff; 0xf55; 0xf5f; 0xff5; 0xfff; |]
  |> Array.map Word.of_int

(** Default character font. *)
let default_font =
  Monitor_font.default

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

  let elapsed_since_blink = now - t.last_blink_time in

  let t =
    if elapsed_since_blink >= blink_period then
      { t with last_blink_time = now; blink_visible = not t.blink_visible }
    else t
  in

  let t =
    match t.state with
    | Starting start_time -> begin
        let elapsed_since_starting = now - start_time in

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
    let open Program in
    let open Program.Functor in
    let open Program.Monad in

    read_register Reg.B >>= fun b ->
    read_register Reg.A |> map Word.to_int >>= function
    | 0 -> begin (* MEM_MAP_SCREEN *)
        Program.Return {
          t with
          video_memory_offset = b;
          state =
            if (b <> word 0) then
              if t.state = Stopped then Starting now else Running
            else Stopped
        }
      end
    | 1 -> begin (* MEM_MAP_FONT *)
        Program.Return { t with font_memory_offset = b }
      end
    | 2 -> begin (* MEM_MAP_PALETTE *)
        Program.Return { t with palette_memory_offset = b }
      end
    | 3 -> begin (* SET_BORDER_COLOR *)
        Program.Return {
          t with
          border_color_index = UInt8.of_int (Word.(b land word 0xf |> to_int))
        }
      end
    | 4 -> begin (* MEM_DUMP_FONT *)
        let rec loop offset =
          if offset >= Array.length default_font then Program.Return t
          else
            let (w1, w2) = default_font.(offset) in
            let v = Word.(b + (word 2 * word offset)) in
            Program.write_memory v w1 >>= fun () ->
            Program.write_memory Word.(v + word 1) w2 >>= fun () ->
            loop (offset + 1)
        in
        loop 0
      end
    | 5 -> begin (* MEM_DUMP_PALETTE *)
        let rec loop offset =
          if offset >= Array.length default_palette then Program.Return t
          else
            Program.write_memory Word.(b + word offset) default_palette.(offset) >>= fun () ->
            loop (offset + 1)
        in
        loop 0
      end
    | _ -> Program.Return t
  end

let on_interaction memory t =
  IO.Monad.(render memory t >>= fun () -> IO.unit t)

let info =
  Device.Info.{
    id = (word 0x7349f615, word 0xf615);
    manufacturer = (word 0x1c6c, word 0x8b36);
    version = word 0x1802
  }
