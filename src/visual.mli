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

  val make : title:string -> width:int -> height:int -> t Lwt.t
end

val set_color : Window.t -> Color.t -> unit Lwt.t

val clear : Window.t -> unit Lwt.t

val rectangle : Window.t -> origin:(int * int) -> width:int -> height:int -> unit Lwt.t

val render : Window.t -> unit Lwt.t
