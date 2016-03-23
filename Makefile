SHELL = bash

STUB_SOURCES = src/emu/clock_precision_stub.c
STUB_OBJECTS = src/emu/clock_precision_stub.o

NATIVE_LINKER_FLAGS = -lflags $(STUB_OBJECTS)
BYTE_LINKER_FLAGS = -lflags -custom,$(STUB_OBJECTS)

SHELL_DIR=shells

OCAMLBUILD = ocamlbuild -use-ocamlfind

default: top

top: emu_top asm_top
	$(OCAMLBUILD) shell.byte
	mv shell.byte run-shell

test: nebula_emu_test libraries_test

install: default uninstall libraries nebula_asm
	ocamlfind install nebula META \
		_build/lib/functional/{functional.cma,functional.cmxa,functional.cmi} \
		_build/lib/properties/{properties.cma,properties.cmxa,properties.cmi} \
		_build/lib/word/{word.cma,word.cmxa,word.cmi} \
		_build/src/asm/{nebula_asm.cma,nebula_asm.cmxa,nebula_asm.cmi}

uninstall:
	ocamlfind remove nebula

# Assembler

asm_top: nebula_asm
	$(OCAMLBUILD) top/asm/nebula_asm.top
	install nebula_asm.top $(SHELL_DIR)/asm
	rm nebula_asm.top

nebula_asm: libraries
	$(OCAMLBUILD) src/asm/nebula_asm.cma
	$(OCAMLBUILD) src/asm/nebula_asm.cmxa

# Emulator

emu_top: nebula_emu
	$(OCAMLBUILD) $(BYTE_LINKER_FLAGS) top/emu/nebula_emu.top
	install nebula_emu.top $(SHELL_DIR)/emu
	rm nebula_emu.top

nebula_emu_test: nebula_emu
	$(OCAMLBUILD) test/emu/emu_spec.byte
	./emu_spec.byte

nebula_emu: stubs libraries
	$(OCAMLBUILD) $(BYTE_LINKER_FLAGS) src/emu/emu.cma
	$(OCAMLBUILD) $(NATIVE_LINKER_FLAGS) src/emu/nebula_emu.native

nebula_emu_profiled: stubs libraries
	$(OCAMLBUILD) $(NATIVE_LINKER_FLAGS) src/emu/nebula_emu.p.native

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
