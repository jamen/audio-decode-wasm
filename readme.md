
# audio-decode-wasm ![expirmental](https://img.shields.io/badge/stability-experimental-red.svg)

**Note:** This doesn't work quite yet, but the structure is drafted out.

A stream that decodes ArrayBuffers into [AudioBuffers](https://github.com/audiojs/audio-buffer).  The decoders are made in [WebAssembly](https://webassembly.org/) so they are portable (Node.js and browser) and decent speed.

```js
const decoder = require('audio-decode-wasm')

// Obtain codec module and initialize decoder:
const mod = await fetch('wav.wasm').then(req => WebAssembly.compileStreaming(res))
const decode = await decoder(mod)

// Decode ArrayBuffers:
decode(arrayBuf, (err, audioBuf) => {
  // ...
})
```

## Install

```sh
npm i -D audio-decode-wasm
```

## Usage

### `decoder(mod) -> Promise<decode>`

Initializes a decoder from a given [`WebAssembly.Module`].  The available ones
can be seen in [`src/`](src/).

```js
const mod = new WebAssembly.Module(...)

decoder(mod).then(decode => {
  // ...
})
```

### `decode(arrayBuffer, done)`

Decodes `arrayBuffer` and calls `done(err, audioBuffer)` when finished.

To stop or "reset" the stream send `decode(null)`.  

```js
const decode = await decoder(mod)

fetch('foo.wav')
.then(res => res.arrayBuffer())
.then(buf => {
  decode(buf, (err, audio) => {
    // ...
  })
})
```

### Using multiple modules

```js
Promise.all([
  decoder(...),
  decoder(...)  
]).then(([ wav, mp3, ... ]) => {
  // ...
})
```

### How does it work?

The decoders are wrote in C and compiled with [Emscripten](https://github.com/kripken/emscripten).  The code is more restricted than a normal Emscripten runtime so it's cheap to load.

Each C modules has the functions

```c
Context* open(unsigned char* input, float* output)
void process(Context* context, int amount)
```

From JS you can create the context with `_open(input, output)`, where the parameters and return values are pointers on WebAssembly's memory, which JS can access and modify.

The `Context` from C looks like:

```c
typedef struct {
  unsigned char* input;
  float* output;
  uint16_t number_of_channels;
  uint32_t sample_rate;
  unsigned char params;
  // ...
} Context;
```

To construct an `AudioBuffer` you need `numberOfChannels` and `sampleRate`, so JS imports a `set_params(int, int)` function which C can call.

The stream routine would look like this:

 1. JS copies `ArrayBuffer` into WebAssembly's input buffer.
 2. WebAssembly decodes it to planar float values on the output buffer.
 3. JS copies the output buffer as `Float32Array`s into an `AudioBuffer`.
 4. Repeat until stream is done.

## Building

Requires [Emscripten](https://github.com/kripken/emscripten), [Binaryen](https://github.com/webassembly), and [WABT](https://github.com/webassembly/wabt).  Then, using `make`:

- `make` to create `dist/`
- `make debug` to produce `dist/*.wat`
- `make clean` to remove output
