(** Pure description of interaction with the computer.

    The important idea behind a {! Program.t} is that they are a pure specification
    of an interaction with the DCPU-16 and its memory without any actual effect
    taking place.

    Programs must be {b executed} against a {! Computer.t} in order to have any
    effect. The {! Computer_state} module executes programs in the
    context of the {! State} monad instantiated with {! Computer.t}, for instance.

    @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

open Functional
open Prelude

module Op : sig
  type 'a t =
    | Read_memory of word * (word -> 'a)
    | Write_memory of word * word * 'a
    | Read_register of Reg.t * (word -> 'a)
    | Write_register of Reg.t * word * 'a
    | Read_special of Special.t * (word -> 'a)
    | Write_special of Special.t * word * 'a
    | Set_flag of Cpu.Flag.t * bool * 'a
    | Get_flag of Cpu.Flag.t * (bool -> 'a)

  val map : ('a -> 'b) -> 'a t -> 'b t
end

include module type of Free.Make(Op)

val read_memory : word -> word t

val write_memory : word -> word -> unit t

val read_register : Reg.t -> word t

val write_register : Reg.t -> word -> unit t

val read_special : Special.t -> word t

val write_special : Special.t -> word -> unit t

val get_flag : Cpu.Flag.t -> bool t

val set_flag : Cpu.Flag.t -> bool -> unit t

(** Read the value pointed to by the program counter and increment the program
    counter. *)
val next_word : word t

(** Push a value onto the stack. *)
val push : word -> unit t

(** Pop a value from the stack. *)
val pop : word t

(** Read a region of memory starting at an offset. *)
val read_extent : word -> int -> word list t
