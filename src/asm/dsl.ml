include Value

module I = Instruction

let add b a = I.Binary (I.Code.Add, b, a)

let a = R Register.A
let b = R Register.B
