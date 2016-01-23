SHELL = bash

STUB_SOURCES = src/emulator/clock_precision_stub.c
STUB_OBJECTS = src/emulator/clock_precision_stub.o

NATIVE_LINKER_FLAGS = -lflags $(STUB_OBJECTS)
BYTE_LINKER_FLAGS = -lflags -custom,$(STUB_OBJECTS)

SHELL_DIR=shells

OCAMLBUILD = ocamlbuild -use-ocamlfind

default: top

top: emulator_top asm_top compiler_top
	$(OCAMLBUILD) shell.byte
	mv shell.byte run-shell

test: nebula_emulator_test libraries_test

install: default uninstall libraries nebula_asm
	ocamlfind install nebula META \
		_build/lib/functional/{functional.cma,functional.cmxa,functional.cmi} \
		_build/lib/properties/{properties.cma,properties.cmxa,properties.cmi} \
		_build/lib/word/{word.cma,word.cmxa,word.cmi} \
		_build/src/asm/{nebula_asm.cma,nebula_asm.cmxa,nebula_asm.cmi}

uninstall:
	ocamlfind remove nebula

# Compiler

compiler_top: nebula_compiler
	$(OCAMLBUILD) top/compiler/nebula_compiler.top
	install nebula_compiler.top $(SHELL_DIR)/compiler
	rm nebula_compiler.top

nebula_compiler: nebula_asm libraries
	$(OCAMLBUILD) src/compiler/nebula_compiler.cma
	$(OCAMLBUILD) src/compiler/nebula_compiler.cmxa

# Assembler

asm_top: nebula_asm
	$(OCAMLBUILD) top/asm/nebula_asm.top
	install nebula_asm.top $(SHELL_DIR)/asm
	rm nebula_asm.top

nebula_asm: libraries
	$(OCAMLBUILD) src/asm/nebula_asm.cma
	$(OCAMLBUILD) src/asm/nebula_asm.cmxa

# Emulator

emulator_top: nebula_emulator
	$(OCAMLBUILD) $(BYTE_LINKER_FLAGS) top/emulator/nebula_emulator.top
	install nebula_emulator.top $(SHELL_DIR)/emulator
	rm nebula_emulator.top

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

# Support libraries

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

# Common targets

.PHONY: clean

clean:
	ocamlbuild -clean
