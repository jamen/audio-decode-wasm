
import AudioBuffer from 'audio-buffer'

export default mod => {
  const memory = new WebAssembly.Memory({ initial: 256 })
  const inputBuffer = new Uint8Array(memory.buffer)
  const CHUNK_SIZE = 0xFFFF * 4
  let numberOfChannels = null
  let sampleRate = null

  return WebAssembly.instantiate(mod, {
    env: {
      _set_params (a, b) {
        numberOfChannels = a
        sampleRate = b
      },
      'g$_CHUNK_SIZE': () => CHUNK_SIZE,
      memory,
      memoryBase: 0,
      table: new WebAssembly.Table({ initial: 10, element: 'anyfunc' }),
      tableBase: 0,
      // TODO: Can these be removed? Memory is being managed manually.
      DYNAMICTOP_PTR: 0,
      ABORT: 0,
      tempDoublePtr: 0,
      abortStackOverflow () {
        throw new Error('abortStackOverflow()')
      }
    },
    'global': { 'NaN': NaN, 'Infinity': Infinity },
    'global.Math': Math
  }).then(instance => {
    instance.exports.__post_instantiate()
    const input = 0xFFFF * 2
    const output = 0xFFFF * 6

    // Open context
    const context = instance.exports._open(input, output)

    function processChunk (chunk) {
      // Process input
      inputBuffer.set(new Uint8Array(chunk), input)
      instance.exports._process(context, chunk.length)
      // Process output
      let length = ~~(CHUNK_SIZE / numberOfChannels)
      let audioBuf = new AudioBuffer({ length, numberOfChannels, sampleRate })
      for (let c = 0; c < numberOfChannels; c++) {
        audioBuf.copyToChannel(new Float32Array(memory.buffer, c * length, length), c)
      }
      return audioBuf
    }

    return function decode (chunk, cb) {
      if (chunk === null) {
        numberOfChannels = null
        sampleRate = null
        return
      } else if (chunk.byteLength > CHUNK_SIZE) {
        cb(null, processChunk(new Uint8Array(chunk, 0, CHUNK_SIZE)))
        decode(chunk.slice(CHUNK_SIZE + 1), cb)
      } else {
        cb(null, processChunk(chunk))
      }
    }
  })
}
