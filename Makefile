SHELL=bash

STUB_SOURCES = src/precision_clock_stub.c
STUB_OBJECTS = src/precision_clock_stub.o

NATIVE_LINKER_FLAGS = -lflags $(STUB_OBJECTS)
BYTE_LINKER_FLAGS = -lflags -custom,$(STUB_OBJECTS)

OCAMLBUILD = ocamlbuild -use-ocamlfind

default: nebula top

# Nebula

top: nebula
	$(OCAMLBUILD) $(BYTE_LINKER_FLAGS) top/nebula.top

test: nebula
	$(OCAMLBUILD) test/nebula_test.byte
	./nebula_test.byte

nebula: stubs libraries
	$(OCAMLBUILD) $(BYTE_LINKER_FLAGS) src/nebula.cma
	$(OCAMLBUILD) $(NATIVE_LINKER_FLAGS) src/nebula_main.native

nebula_profiled: stubs
	$(OCAMLBUILD) $(NATIVE_LINKER_FLAGS) src/nebula_main.p.native

stubs: $(STUB_SOURCES)
	$(OCAMLBUILD) $(STUB_OBJECTS)

doc: lib
	mkdir -p doc

	ocamlfind ocamldoc \
		-I _build/src \
		-I _build/src/devices \
    -I _build/lib \
		-I _build/lib/functional \
		-colorize-code \
		-html -d doc \
		-package cmdliner \
		-package ppx_deriving.std \
		-package tsdl \
		$(shell echo src/*.{mli,ml} src/devices/*.ml lib/functional/*.{mli,ml})

# Support libraries

libraries: functional properties

functional:
	$(OCAMLBUILD) lib/functional.cma
	$(OCAMLBUILD) lib/functional.cmxa

properties: functional
	$(OCAMLBUILD) lib/properties.cmo
	$(OCAMLBUILD) lib/properties.cmx

.PHONY: clean

clean:
	rm -rf doc
	ocamlbuild -clean
