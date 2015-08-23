open Functional

open Ctypes
open Unsigned

open IO.Monad

module Color = struct
  type t = Sdl.color

  let make ~r ~g ~b =
    Sdl.Color.create ~r:(UInt8.to_int r) ~g:(UInt8.to_int g) ~b:(UInt8.to_int b) ~a:255

  let red =
    make ~r:(UInt8.of_int 255) ~g:UInt8.zero ~b:UInt8.zero

  let green =
    make ~r:UInt8.zero ~g:(UInt8.of_int 255) ~b:UInt8.zero

  let blue =
    make ~r:UInt8.zero ~g:UInt8.zero ~b:(UInt8.of_int 255)

  let black =
    make UInt8.zero UInt8.zero UInt8.zero

  let white =
    make (UInt8.of_int 255) (UInt8.of_int 255) (UInt8.of_int 255)
end

module Window = struct
  type t = {
    window : Sdl.window;
    renderer : Sdl.renderer;
  }

  let make ~title ~width ~height =
    Sdl.lift "Creating window: %s" (fun () ->
         Sdl.create_window ~w:width ~h:height title Sdl.Window.windowed) >>= fun window ->
    Sdl.lift "Creating renderer: %s" (fun () -> Sdl.create_renderer window) >>= fun renderer ->
    IO.unit { window; renderer }
end

let set_color window color =
  let open Window in
  Sdl.lift "Setting drawing color: %s" (fun () ->
      Sdl.set_render_draw_color
        window.renderer
        (Sdl.Color.r color)
        (Sdl.Color.g color)
        (Sdl.Color.b color)
        255)

let clear window =
  Sdl.lift "Clearing window: %s" (fun () ->
      Sdl.render_clear window.Window.renderer)

let rectangle window ~origin ~width ~height =
  Sdl.lift "Drawing rectangle: %s" begin fun () ->
    let r = Sdl.Rect.create ~x:(fst origin) ~y:(snd origin) ~h:height ~w:width in
    Sdl.render_fill_rect window.Window.renderer (Some r)
  end

let render w =
  IO.lift (fun () -> Sdl.render_present w.Window.renderer)
