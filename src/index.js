
import AudioBuffer from 'audio-buffer'

export default mod => {
  let memory = new WebAssembly.Memory({ initial: 256 })
  let numberOfChannels = null
  let sampleRate = null

  return WebAssembly.instantiate(mod, {
    env: {
      _debug (i) { console.log(i) },
      _set_params (a, b) {
        console.log('got params', a, b)
        numberOfChannels = a
        sampleRate = b
      },
      memory,
      memoryBase: 0,
      table: new WebAssembly.Table({ initial: 10, element: 'anyfunc' }),
      tableBase: 0,
      abortStackOverflow () { throw new Error('aborted') },
    },
    'global': { 'NaN': NaN, 'Infinity': Infinity },
    'global.Math': Math
  }).then(instance => {
    const { _open, _process, __post_instantiate } = instance.exports

    __post_instantiate()

    // Open context
    const inputPtr = 0xFFFF * 2
    const outputPtr = 0xFFFF * 6
    const maximum = 0xFFFF * 4
    const context = _open(inputPtr, outputPtr, maximum)

    const input = new Uint8Array(memory.buffer, inputPtr)
    const output = new Float32Array(memory.buffer, outputPtr + (outputPtr % 4))

    return function decode (chunk, cb) {
      let remainder = null

      if (chunk.byteLength > maximum) {
        remainder = chunk.slice(maximum + 1)
        chunk = new Uint8Array(chunk, 0, maximum)
      } else {
        chunk = new Uint8Array(chunk)
      }

      input.set(chunk)
      _process(context, 0, chunk.length)

      let buf = new AudioBuffer({
        length: chunk.length,
        numberOfChannels,
        sampleRate
      })

      const block = (chunk.length / numberOfChannels) / 4
      for (let channel = 0; channel < numberOfChannels; channel++) {
        buf.copyToChannel(output.subarray(channel * block, block), channel)
      }

      cb(null, buf)

      if (remainder) {
        return decode(remainder, cb)
      }
    }
  })
}
