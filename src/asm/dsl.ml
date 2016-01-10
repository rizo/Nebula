include Value

include Dcpu16.Functor
include Dcpu16.Monad
include Dcpu16.Applicative

let w = Word.of_int

let push = Push
let pop = Pop
let peek = Peek
let pick i = Pick (w i)

let ra = R Reg.A
let rb = R Reg.B

let imm i = I (w i)
let addr i = A (imm i)
let at a = A a

let add b a = Inst.(Binary (Code.Add, b, a))

let ( >> ) x y =
  x >>= fun () -> y

let void t =
  t >>= fun _ -> Dcpu16.unit ()
