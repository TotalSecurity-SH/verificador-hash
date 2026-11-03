SHELL := /bin/bash
# Compila los algoritmos de hash a WebAssembly sin libc ni Emscripten.
CC      := clang
SRC     := src/md5.c src/sha1.c src/sha256.c src/sha512.c src/util.c
WASM    := web/hash/hash.wasm

WASM_FLAGS := --target=wasm32 -O3 -nostdlib -fno-builtin -ffreestanding \
              -Wall -Wextra -Wl,--no-entry -Wl,--export-dynamic -Wl,--strip-all \
              -Wl,--initial-memory=4194304

.PHONY: all wasm test test-wasm clean

all: wasm test test-wasm

wasm: $(WASM)

$(WASM): $(SRC) src/wasm.c src/hash.h
	$(CC) $(WASM_FLAGS) -o $@ $(SRC) src/wasm.c

# Pruebas nativas: vectores oficiales + comparación con sha*sum del sistema.
test:
	$(CC) -O2 -Wall -Wextra -o tests/run tests/test_vectors.c $(SRC:src/util.c=)
	./tests/run
	@head -c 3000000 /dev/urandom > tests/random.bin
	@./tests/run tests/random.bin > tests/out.txt
	@md5sum tests/random.bin | cut -d' ' -f1 | diff - <(sed -n 1p tests/out.txt) && echo "OK   md5 = md5sum"
	@sha1sum tests/random.bin | cut -d' ' -f1 | diff - <(sed -n 2p tests/out.txt) && echo "OK   sha1 = sha1sum"
	@sha256sum tests/random.bin | cut -d' ' -f1 | diff - <(sed -n 3p tests/out.txt) && echo "OK   sha256 = sha256sum"
	@sha512sum tests/random.bin | cut -d' ' -f1 | diff - <(sed -n 4p tests/out.txt) && echo "OK   sha512 = sha512sum"
	@rm -f tests/random.bin tests/out.txt

# Prueba el hash.wasm compilado (Node) contra md5sum y sha*sum.
test-wasm: $(WASM)
	node tests/wasm_test.mjs

clean:
	rm -f tests/run $(WASM)
