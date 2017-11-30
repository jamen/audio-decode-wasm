PATH := $(PATH):node_modules/.bin
SHELL := /bin/bash

build: clean
	mkdir -p dist
	emcc src/wav.c -s WASM=1 -s SIDE_MODULE=1 -o dist/wav.wasm
	wasm-opt -Oz dist/wav.wasm
	rollup src/index.js -o dist/audio-decode-wasm.cjs.js -f cjs -c
	rollup src/index.js -o dist/audio-decode-wasm.es.js -f es -c

debug: build
	wasm2wat dist/wav.wasm > dist/wav.wat

clean:
	rm -rf dist
