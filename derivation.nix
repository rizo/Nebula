{stdenv, libffi, SDL2}:

stdenv.mkDerivation {
  name = "ocaml-nebula";
  version = "0.1.0";

  buildInputs = [ libffi SDL2 ];

  src = ./.;

  installPhase = "make PREFIX=$out NAME=$name install";
}
