(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Functional

open Ctypes
open Unsigned

open IO.Monad

module Color = struct
  type t = Sdl.color

  let make ~r ~g ~b =
    UInt8.(Sdl.Color.create ~r:(to_int r) ~g:(to_int g) ~b:(to_int b) ~a:255)

  let red =
    UInt8.(make ~r:(of_int 255) ~g:zero ~b:zero)

  let green =
    UInt8.(make ~r:zero ~g:(of_int 255) ~b:zero)

  let blue =
    UInt8.(make ~r:zero ~g:zero ~b:(of_int 255))

  let black =
    UInt8.(make zero zero zero)

  let white =
    UInt8.(make (of_int 255) (of_int 255) (of_int 255))
end

module Window = struct
  type t = {
    window : Sdl.window;
    renderer : Sdl.renderer;
  }

  let make ~title ~width ~height =
    Sdl.lift "Creating window: %s" begin fun () ->
      Sdl.create_window ~w:width ~h:height title Sdl.Window.windowed
    end >>= fun window ->
    Sdl.lift "Creating renderer: %s" begin fun () ->
      Sdl.create_renderer window
    end >>= fun renderer -> IO.unit { window; renderer }
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
