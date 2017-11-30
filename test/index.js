
const decoder = require('../dist/audio-decode-wasm.cjs')
const { join } = require('path')
const { promisify } = require('util')
const { readFile } = require('fs')

const read = promisify(readFile)

Promise.all([
  read(join(__dirname, '../dist/wav.wasm')),
  read(join(__dirname, 'lena.wav'))
]).then(([ mod, wav ]) => {
  WebAssembly.compile(mod.buffer)
  .then(mod => decoder(mod))
  .then(decode => {
    decode(wav.buffer, (err, data) => {
      console.log(data)
    })
  })
})
