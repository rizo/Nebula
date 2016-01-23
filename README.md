# Nebula

![](https://github.com/hakuch/Nebula/blob/master/img/monitor-animation.gif)

Nebula is a complete emulator and assembler for the fictional DCPU-16 computer, including its hardware
peripherals.

More information on the assembler, `nebula-asm`, can be found [here](https://github.com/hakuch/Nebula/tree/master/src/asm).

Nebula is a part of [DCPU-16 Universe](https://github.com/hakuch/Dcpu16Universe).

## Philosophy

Nebula is written in OCaml, and in a very pure functional style.

Some notable points:

- Nebula includes a fairly extensive library for functional programming, `Functional`, consisting of several common monads (including the free monad and monad transformers) and functional data structures.

- Nebula includes a complete property-based testing library called `Properties` that is also used for unit-testing.

- All impure code is wrapped in `IO.t`, which is analogous to `IO` in Haskell. The implementation of the `IO` module is also included and has support for exceptions and both functor and monad interfaces.

- Interaction with the CPU and memory is expressed as pure data with a DSL defined using the free monad. These "programs" can be subsequently _interpreted_ in terms of the state monad or any other context.

### Correctness vs performance

The implementation of Nebula focuses on being correct and understandable. Particularly since Nebula uses immutable structures and functional idioms throughout, this comes at the expense of a focus on performance.

Nebula is frequently profiled to identify when the performance cost of these idioms is not acceptable, and this has been a successful approach thus far.

# Quick-start guide

## Installing through opam

The fastest way to use Nebula is to install it via opam:

```bash
$ opam install nebula
```

Alternatively, you can download Nebula's source code and compile it yourself.

## Installing from source

Nebula is written in OCaml and depends on some packages available in opam. Start by grabbing the Nebula sources:

```bash
$ git clone git@github.com:hakuch/Nebula.git
```

Then register the development version with opam:

```bash
$ opam pin add nebula .
```

and finally install the `nebula` executable with

```bash
$ opam install nebula
```

Subsequent changes (perhaps made during development) can be installed by invoking

```bash
$ opam upgrade nebula
```

## Running

Nebula consists of two executables: `nebula-asm` and `nebula-emulator`. Both have a command-line interface.

## Development

In addition to requiring the opam package manager, Nebula depends on two packages being installed on your system:

- SDL2
- libffi

### Building

Nebula is built by invoking the `make` command with one of the following targets:

- `top`: Interactive OCaml top-level REPL for `nebula-asm` and `nebula-emulator`
- `libraries`: Nebula's support libraries.
- `test`: Unit tests (also runs tests)
- `nebula_emulator`: The `nebula-emulator` program and libraries
- `nebula_asm`: The `nebula-asm` program and libraries

### Testing

Build and run tests with `make test`.

Nebula tests used to be written with the `oUnit` package, but that is being phased-out in favour of Nebula's home-grown property-based testing library, `Properties`.

### Interactive read-eval-print-loop (REPL)

Nebula comes with an interactive REPL built on top of [utop](https://opam.ocaml.org/blog/about-utop/) that starts with all modules loaded and with extra pretty-printers for Nebula types installed.

Once built with `make top`, REPLs exist for both development of `nebula-emulator` and for `nebula-asm`.

The REPLs for `nebula-asm` and `nebula-emulator` can be invoked as

```bash
$ ./run-shell asm
```

and

```bash
$ ./run-shell emulator
```

respectively.

## Changelog

See [CHANGELOG.md](CHANGELOG.md).
