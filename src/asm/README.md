# nebula-asm

`nebula-asm` is an embedded domain-specific language (EDSL) for assembling programs to run on the DCPU-16.

Like `nebula-emulator`, `nebula-asm` is written in a pure-functional style in OCaml.

Rather than write DCPU-16 programs in a dedicated language which requires its own parser and file-handling, `nebula-asm` allows programs to be directly specified in OCaml.

## Example

As an example, the file `loop.ml` consists of the following contents:

```ocaml
open Nebula_asm.Dsl

let _ = assembly_main "a.bin" begin
    label >>= fun loop ->
    set pc (imm loop)
  end
```

This assembly program -- the body between `begin` and `end` -- simply loops forever.

Once the Nebula package is installed (see the [main README](https://github.com/hakuch/Nebula) for instructions), invoking

```bash
$ nebula-asm loop.ml
```

will first compile the OCaml program to an executable bytecode file `loop.byte`, and then execute it. Executing the program encodes the DCPU-16 assembly and writes the DCPU-16 bytecode to a file `a.bin`.
