(** @author Jesse Haber-Kucharsky
    @see 'LICENSE' License details *)

module Context = struct
  type t = Assemble | Link
end

type t =
  | Fixed of Context.t * Word.t
  | Unresolved
