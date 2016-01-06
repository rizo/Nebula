SHELL = bash

STUB_SOURCES = src/emulator/clock_precision_stub.c
STUB_OBJECTS = src/emulator/clock_precision_stub.o

NATIVE_LINKER_FLAGS = -lflags $(STUB_OBJECTS)
BYTE_LINKER_FLAGS = -lflags -custom,$(STUB_OBJECTS)

OCAMLBUILD = ocamlbuild -use-ocamlfind

default: nebula top

# Nebula

top: nebula
	$(OCAMLBUILD) $(BYTE_LINKER_FLAGS) top/emulator/nebula.top

test: nebula libraries_test
	$(OCAMLBUILD) test/emulator/nebula_spec.byte
	./nebula_spec.byte

nebula: stubs libraries
	$(OCAMLBUILD) $(BYTE_LINKER_FLAGS) src/emulator/nebula.cma
	$(OCAMLBUILD) $(NATIVE_LINKER_FLAGS) src/emulator/nebula_main.native

nebula_profiled: stubs libraries
	$(OCAMLBUILD) $(NATIVE_LINKER_FLAGS) src/emulator/nebula_main.p.native

stubs: $(STUB_SOURCES)
	$(OCAMLBUILD) $(STUB_OBJECTS)

# Support libraries.

libraries_test: functional_test word_test

libraries: functional properties word

functional_test: functional
	$(OCAMLBUILD) lib/functional/test/functional_spec.byte
	./functional_spec.byte

word_test: word
	$(OCAMLBUILD) lib/word/test/word_spec.byte
	./word_spec.byte

functional:
	$(OCAMLBUILD) lib/functional/functional.cma
	$(OCAMLBUILD) lib/functional/functional.cmxa

properties: functional
	$(OCAMLBUILD) lib/properties/properties.cma
	$(OCAMLBUILD) lib/properties/properties.cmxa

word: functional
	$(OCAMLBUILD) lib/word/word.cma
	$(OCAMLBUILD) lib/word/word.cmxa

.PHONY: clean

clean:
	ocamlbuild -clean
