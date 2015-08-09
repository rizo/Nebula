# Nebula

Nebula is an emulator for the fictional DCPU-16 computer, including its hardware
peripherals.

## Philosophy

Nebula is written in a very pure functional style that is not particularly common in OCaml.

Some notable points:

- Most impure code is managed in a `IO.t` context with a monadic interface that is very similar to `IO` in Haskell. The vast majority of the code is purely functional and interfaces that expose mutation are (virtually) absent.
- Interaction with the CPU and memory is expressed as pure data with a DSL defined using the free monad. These "programs" can be subsequently _interpreted_ in terms of the state monad or any other context.

## About

Nebula is written by Jesse Haber-Kucharsky (@hakuch).

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

## Development

### Nix environment

Nebula depends on a few external libraries that are not tracked in opam. Nebula uses the [Nix](https://nixos.org/nix/) package manager to manage these dependencies with the `derivation.nix` and `default.nix` files.

Executing

```bash
$ nix-shell
```

will start a new shell with the necessary dependencies in scope (after they have been downloaded, if necessary).

### Building

Nebula is built by invoking the `make` command with one of the following targets:

- `doc`: Development documentation
- `top`: Interactive OCaml top-level REPL for Nebula's sources
- `lib`: The Nebula library that is used for the top-level, the main executable, and tests
- `test`: Build unit tests.
- `nebula`: The main executable.

The `Makefile` actually calls into a short script, `make.ml`, that populates flags for the `ocamlbuild` tool.

### Testing

After building the tests with `make test`, run the tests with

```bash
$ ./nebula_test.byte
```

Unit tests are written with the `ounit` package and ideally cover all functionality.
