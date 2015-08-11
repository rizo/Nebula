STUB_SOURCES = src/precision_clock_stub.c
STUB_OBJECTS = src/precision_clock_stub.o

NATIVE_LINKER_FLAGS = -lflags $(STUB_OBJECTS)
BYTE_LINKER_FLAGS = -lflags -custom,$(STUB_OBJECTS)

OCAMLBUILD = ocamlbuild -use-ocamlfind

default: nebula top

nebula: lib
	$(OCAMLBUILD) $(NATIVE_LINKER_FLAGS) src/nebula_main.native

stubs: $(STUB_SOURCES)
	$(OCAMLBUILD) $(STUB_OBJECTS)

lib: stubs
	$(OCAMLBUILD) $(BYTE_LINKER_FLAGS) src/nebula.cma

test: lib
	$(OCAMLBUILD) test/nebula_test.byte

top: lib
	$(OCAMLBUILD) $(BYTE_LINKER_FLAGS) top/nebula.top

# TODO
# doc: lib
# 	mkdir -p doc
# 	ocaml make.ml doc

.PHONY: clean

clean:
	rm -rf doc
	ocamlbuild -clean
