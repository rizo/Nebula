open Nebula_asm.Dsl

module Monitor = struct
  let index = 2
  let memory = 0x4000
  let font_base = 0xf000

  let activate =
    set ra (imm 0) >>
    set rb (imm memory) >>
    hwi (imm index)
end

let data =
  define "indices" (asciip "0123456789abcdef")

let busy_loop =
  label >>= fun loop ->
  set pc (imm loop)

let _ = assembly_main "a.bin" begin
    Monitor.activate >>

    set ri (imm 0) >>
    set rj (ind @@ L "indices") >>

    label >>= fun loop -> begin
      (* Set the foreground color and character. *)
      set rx (imm Monitor.font_base) >>
      bor rx (off (L "indices") ri) >>

      (* Set the background color. *)
      set ry ri >>
      shl ry (imm 8) >>
      bor rx ry >>

      set (off (imm @@ Monitor.memory - 1) ri) rx >>
      add ri (imm 1) >>
      ifn ri rj >>
      set pc (imm loop)
    end >>

    busy_loop >>
    data
  end
