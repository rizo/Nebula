SHELL=bash

STUB_SOURCES = src/precision_clock_stub.c
STUB_OBJECTS = src/precision_clock_stub.o

NATIVE_LINKER_FLAGS = -lflags $(STUB_OBJECTS)
BYTE_LINKER_FLAGS = -lflags -custom,$(STUB_OBJECTS)

OCAMLBUILD = ocamlbuild -use-ocamlfind

default: nebula top

nebula: stubs
	$(OCAMLBUILD) $(NATIVE_LINKER_FLAGS) src/nebula_main.native

profiled: stubs
	$(OCAMLBUILD) $(NATIVE_LINKER_FLAGS) src/nebula_main.p.native

test: lib
	$(OCAMLBUILD) test/nebula_test.byte
	./nebula_test.byte

top: lib
	$(OCAMLBUILD) $(BYTE_LINKER_FLAGS) top/nebula.top

lib: stubs
	$(OCAMLBUILD) $(BYTE_LINKER_FLAGS) src/nebula.cma

stubs: $(STUB_SOURCES)
	$(OCAMLBUILD) $(STUB_OBJECTS)

doc: lib
	mkdir -p doc

	ocamlfind ocamldoc \
		-I _build/src \
		-I _build/src/functional \
		-I _build/src/devices \
		-html -d doc \
		-package cmdliner \
		-package tsdl \
		$(shell echo src/*.{mli,ml} src/devices/*.ml src/functional/*.{mli,ml})

.PHONY: clean

clean:
	rm -rf doc
	ocamlbuild -clean
