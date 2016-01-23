open Functional

module M = Machine

module D = Nebula_asm.Dcpu16
module L = Nebula_asm.Dsl

let rec synthesize m =
  let open D.Monad in

  match m with
  | M.Suspend (M.Op.Push (x, n)) -> L.(set push (imm x)) >>= fun () -> synthesize n
  | M.Suspend (M.Op.Pop n) -> L.(set rx pop) >>= fun () -> synthesize n
  | M.Suspend (M.Op.Peek (i, n)) -> L.(set push (pick i)) >>= fun () -> synthesize n
  | M.Suspend (M.Op.Apply (f, n)) -> begin
      L.(set ry pop >>
         set rx pop) >>= fun () ->
      begin
        match f with
        | M.Arithmetic.Add -> L.(add rx ry)
        | M.Arithmetic.Multiply -> L.(mul rx ry)
      end >>= fun () ->
      L.(set push rx)
    end >>= fun () ->
    synthesize n
  | M.Suspend (M.Op.Swap n) -> begin
      L.(set rx (pick 1) >>
         set (pick 1) peek >>
         set peek rx) >>= fun () ->
      synthesize n
    end
  | M.Return () -> D.unit ()
