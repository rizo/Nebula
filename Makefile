SHELL=bash

STUB_SOURCES = src/clock_precision_stub.c
STUB_OBJECTS = src/clock_precision_stub.o

NATIVE_LINKER_FLAGS = -lflags $(STUB_OBJECTS)
BYTE_LINKER_FLAGS = -lflags -custom,$(STUB_OBJECTS)

OCAMLBUILD = ocamlbuild -use-ocamlfind

default: nebula top

# Nebula

top: nebula
	$(OCAMLBUILD) $(BYTE_LINKER_FLAGS) top/nebula.top

test: nebula libraries_test
	$(OCAMLBUILD) test/nebula_test.byte
	./nebula_test.byte

nebula: stubs libraries
	$(OCAMLBUILD) $(BYTE_LINKER_FLAGS) src/nebula.cma
	$(OCAMLBUILD) $(NATIVE_LINKER_FLAGS) src/nebula_main.native

nebula_profiled: stubs libraries
	$(OCAMLBUILD) $(NATIVE_LINKER_FLAGS) src/nebula_main.p.native

stubs: $(STUB_SOURCES)
	$(OCAMLBUILD) $(STUB_OBJECTS)

# Support libraries.

libraries_test: functional_test

libraries: functional properties

functional_test: functional
	$(OCAMLBUILD) lib/functional/test/functional_spec.byte
	./functional_spec.byte

functional:
	$(OCAMLBUILD) lib/functional/functional.cma
	$(OCAMLBUILD) lib/functional/functional.cmxa

properties: functional
	$(OCAMLBUILD) lib/properties/properties.cma
	$(OCAMLBUILD) lib/properties/properties.cmxa

.PHONY: clean

clean:
	ocamlbuild -clean
