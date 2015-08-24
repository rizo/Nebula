# Nebula

![](https://github.com/hakuch/Nebula/blob/ocaml/img/monitor-animation.gif)

Nebula is an emulator for the fictional DCPU-16 computer, including its hardware
peripherals.

## Philosophy

Nebula is written in OCaml, and in a very pure functional style.

Some notable points:

- All impure code is wrapped in `IO.t`, which is analogous to `IO` in Haskell. The implementation of the `IO` module is also included and has support for exceptions and both functor and monad interfaces.

- Interaction with the CPU and memory is expressed as pure data with a DSL defined using the free monad. These "programs" can be subsequently _interpreted_ in terms of the state monad or any other context.

### Correctness vs performance

First and foremost, the implementation of Nebula focuses on being correct and understandable. Particularly since Nebula uses immutable structures and functional idioms throughout, this comes at the expense of a focus on performance.

Nebula is frequently profiled to identify when the performance cost of these idioms is not acceptable, and this has been a successful approach thus far.

# Quick-start guide

## Getting the source code

Nebula is written in OCaml and depends on some packages available in opam. Start by grabbing the Nebula sources:

```bash
$ git clone git@github.com:hakuch/Nebula.git
```

Make sure to switch to the `ocaml` branch:

```bash
$ git checkout ocaml
```

## Running

Nebula runs from the command line.

Executing

```bash
$ nebula FILE
```

will treat the file named `FILE` as a memory image for the DCPU-16 and start executing instructions at the start of the memory it has loaded.

For more options and information, execute

```bash
$ nebula --help
```

## Installing

The fastest way to use Nebula is to install it to your local file system with opam. A good way to do this is to pin it in the root source directory:

```bash
$ opam pin add nebula .
```

then install it with

```bash
$ opam install nebula
```

This will install the `nebula` executable.

Subsequent changes (perhaps made during development) can be installed by invoking

```bash
$ opam upgrade nebula
```

## Development

In addition to requiring the opam package manager, Nebula depends on two packages being installed on your system:

- SDL2
- libffi

### Building

Nebula is built by invoking the `make` command with one of the following targets:

- `doc`: Development documentation.
- `top`: Interactive OCaml top-level REPL for Nebula's sources.
- `lib`: The Nebula library that is used for the top-level, the main executable, and tests.
- `test`: Build unit tests.
- `nebula`: The main executable.

### Testing

Build and run tests with `make test`. The tests can also be run after they have been built by invoking

```bash
$ ./nebula_test.byte
```

Unit tests are written with the `ounit` package and ideally cover all functionality.

### Interactive read-eval-print-loop (REPL)

Nebula comes with an interactive REPL built on top of [utop](https://opam.ocaml.org/blog/about-utop/) that starts with all modules loaded and with extra pretty-printers for Nebula types installed.

To build the top-level, execute

```bash
$ make top
```

Once it has built, the top-level can be started by invoking

```bash
$ ./nebula.top
```
