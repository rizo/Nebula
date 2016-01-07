include Value

let add b a = Inst.(Binary (Code.Add, b, a))

let a = R Reg.A
let b = R Reg.B
