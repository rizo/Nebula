SHELL = bash

STUB_SOURCES = src/emulator/clock_precision_stub.c
STUB_OBJECTS = src/emulator/clock_precision_stub.o

NATIVE_LINKER_FLAGS = -lflags $(STUB_OBJECTS)
BYTE_LINKER_FLAGS = -lflags -custom,$(STUB_OBJECTS)

OCAMLBUILD = ocamlbuild -use-ocamlfind

default: nebula_emulator top

top: emulator_top asm_top

# Assembler

asm_top:
	$(OCAMLBUILD) top/asm/nebula_asm.top
	mv nebula_asm.top shell/asm

# Emulator

emulator_top: nebula_emulator
	$(OCAMLBUILD) $(BYTE_LINKER_FLAGS) top/emulator/nebula_emulator.top
	mv nebula_emulator.top shell/emulator

test: nebula_emulator_test libraries_test

nebula_emulator_test: nebula_emulator
	$(OCAMLBUILD) test/emulator/emulator_spec.byte
	./emulator_spec.byte

nebula_emulator: stubs libraries
	$(OCAMLBUILD) $(BYTE_LINKER_FLAGS) src/emulator/emulator.cma
	$(OCAMLBUILD) $(NATIVE_LINKER_FLAGS) src/emulator/nebula_emulator.native

nebula_emulator_profiled: stubs libraries
	$(OCAMLBUILD) $(NATIVE_LINKER_FLAGS) src/emulator/nebula_emulator.p.native

stubs: $(STUB_SOURCES)
	$(OCAMLBUILD) $(STUB_OBJECTS)

# Support libraries.

libraries_test: functional_test word_test

libraries: functional properties word

functional_test: functional
	$(OCAMLBUILD) lib/functional/test/functional_spec.byte
	./functional_spec.byte

functional:
	$(OCAMLBUILD) lib/functional/functional.cma
	$(OCAMLBUILD) lib/functional/functional.cmxa

word_test: word
	$(OCAMLBUILD) lib/word/test/word_spec.byte
	./word_spec.byte

word: functional
	$(OCAMLBUILD) lib/word/word.cma
	$(OCAMLBUILD) lib/word/word.cmxa

properties: functional
	$(OCAMLBUILD) lib/properties/properties.cma
	$(OCAMLBUILD) lib/properties/properties.cmxa

.PHONY: clean

clean:
	ocamlbuild -clean
