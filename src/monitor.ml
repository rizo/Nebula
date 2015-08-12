open Prelude

type t = {
  window : Visual.Window.t;
  connected : bool;
  last_blink_time : int;
  blink_visible : bool;
  border_color_offset : word;
  video_color_offset : word;
  font_memory_offset : word;
  palette_memory_offset : word;
}

let width =
  640

let height =
  480

let cells_per_monitor_width =
  32

let cells_per_monitor_height =
  12

let cell_width =
  width / cells_per_monitor_width

let cell_height =
  height / cells_per_monitor_height

let atom_size =
  cell_width / 4

let border_width =
  2 * atom_size

let border_height =
  2 * atom_size

let total_width =
  width + (2 * border_width)

let total_height =
  height + (2 * border_height)

let make window =
  let open IO.Functor in
  Precision_clock.get_time |> map (fun now ->
  { window;
    connected = false;
    last_blink_time = now;
    blink_visible = true;
    border_color_offset = word 0;
    video_color_offset = word 0;
    font_memory_offset = word 0;
    palette_memory_offset = word 0;
  })

let on_tick t =
  IO.unit (t, None)

let on_interrupt message t =
  Program.Return t

let draw_border window =
  IO.Monad.sequence_unit Visual.[
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

let render t =
  let window = t.window in
  IO.Monad.sequence_unit Visual.[
    set_color window Color.black;
    clear window;
    set_color window Color.blue;
    draw_border window;
    render window
  ]

let on_interaction t =
  IO.Monad.(render t >>= fun () -> IO.unit t)

let info =
  Device.Info.{
    id = (word 0x7349f615, word 0xf615);
    manufacturer = (word 0x1c6c, word 0x8b36);
    version = word 0x1802
  }
