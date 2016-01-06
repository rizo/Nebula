(** Simple graphics.

    Drawing commands can be imperatively applied to a buffer, but nothing is
    rendered to the {! Window} until {! render} is invoked.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Functional

open Ctypes
open Unsigned

module Color : sig
  type t

  val make : r:uint8 -> g:uint8 -> b:uint8 -> t

  val red : t

  val green : t

  val blue : t

  val white : t

  val black : t
end

module Window : sig
  type t

  (** Create a new graphical window on which to draw.

      Both the [width] and [height] are expressed in real pixels. *)
  val make : title:string -> width:int -> height:int -> t IO.t
end

(** Set the drawing color for subsequent drawing commands. *)
val set_color : Window.t -> Color.t -> unit IO.t

(** Fill the entire window with the drawing color. *)
val clear : Window.t -> unit IO.t

(** Fill a rectangle. *)
val rectangle : Window.t -> origin:(int * int) -> width:int -> height:int -> unit IO.t

(** Render the buffer to the window. *)
val render : Window.t -> unit IO.t
