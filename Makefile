default: nebula

nebula: lib
	ocaml make.ml nebula

lib:
	ocaml make.ml lib

test: lib
	ocaml make.ml test

top: lib
	ocaml make.ml top

doc: lib
	mkdir -p doc
	ocaml make.ml doc

.PHONY: clean

clean:
	rm -rf doc
	ocamlbuild -clean
