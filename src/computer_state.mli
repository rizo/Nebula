(** {! Computer} operations interpretted in the context of the {! State} monad. *)

open Functional
open Prelude

include module type of State.Make(Computer)

val read_register : Reg.t -> word t

val write_register : Reg.t -> word -> unit t

val read_special : Special.t -> word t

val write_special : Special.t -> word -> unit t

val get_flag : Cpu.Flag.t -> bool t

val set_flag : Cpu.Flag.t -> bool -> unit t

val read_memory : word -> word t

val write_memory : word -> word -> unit t

(** Interpret a {! Program} in the {! Computer_state} context. *)
val of_program : 'a Program.t -> 'a t
