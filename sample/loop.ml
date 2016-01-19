open Nebula_asm.Dsl

let _ = assembly_main "a.bin" begin
    label >>= fun loop ->
    set pc (imm loop)
  end
