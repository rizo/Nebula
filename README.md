# Nebula

Nebula is an emulator for the fictional DCPU-16 computer, including its hardware
peripherals.

## Philosophy

Nebula is written in OCaml, and in a very pure functional style.

Some notable points:

- All impure code is wrapped in `Lwt.t` from the `lwt` package, which is similar to `IO` in Haskell and with support for concurrency. The vast majority of the code is purely functional and interfaces that expose mutation are (virtually) absent.
- Interaction with the CPU and memory is expressed as pure data with a DSL defined using the free monad. These "programs" can be subsequently _interpreted_ in terms of the state monad or any other context.

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

### Nix environment

Nebula depends on a few external libraries that are not tracked in opam. Nebula uses the [Nix](https://nixos.org/nix/) package manager to manage these dependencies with the `derivation.nix` and `default.nix` files.

Executing

```bash
$ nix-shell
```

will start a new shell with the necessary dependencies in scope (after they have been downloaded, if necessary).

### Building

Nebula is built by invoking the `make` command with one of the following targets:

- `doc`: Development documentation.
- `top`: Interactive OCaml top-level REPL for Nebula's sources.
- `lib`: The Nebula library that is used for the top-level, the main executable, and tests.
- `test`: Build unit tests.
- `nebula`: The main executable.

### Testing

After building the tests with `make test`, run the tests with

```bash
$ ./nebula_test.byte
```

Unit tests are written with the `ounit` package and ideally cover all functionality.
