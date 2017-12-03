
#include <stdint.h>
#include <endian.h>

#define WAVE_FORMAT_PCM 0x0001
#define WAVE_FORMAT_FLOAT 0x0003
#define WAVE_FORMAT_ALAW 0x0006
#define WAVE_FORMAT_MULAW 0x0007
#define WAVE_FORMAT_EXT 0xFFFE

#define float_u8 256.0
#define float_s16 32768.0
#define float_s32 214748364.0

#define util_read_int32(i,o) (i[o]|(i[o+1]<<8)|(i[o+2]<<16)|(i[o+3]<<24))
#define util_read_int16(i,o) (i[o]|(i[o+1]<<8))
#define util_read_uint32(i,o) (util_read_int16(i,o)-float_s16)
#define util_read_uint16(i,o) (util_read_int16(i,o)-float_s16)


extern void debug(int, double);
extern unsigned char set_params(int, int);

struct Decoder {
  unsigned char* input;
  double* output;
  int maximum;
  int format;
  int number_of_channels;
  int sample_rate;
  int bits_per_sample;
  int block_align;
  unsigned char did_header;
};

struct Decoder* open(unsigned char* input, double* output, int maximum);
void process(struct Decoder* decoder, int offset, int length);
void process_pcm(struct Decoder* decoder, int offset, int length);
void process_float(struct Decoder* decoder, int offset, int length);
void process_alaw(struct Decoder* decoder, int offset, int length);
void process_mulaw(struct Decoder* decoder, int offset, int length);
void process_ext(struct Decoder* decoder, int offset, int length);

struct Decoder* open(unsigned char* input, double* output, int maximum) {
  struct Decoder* decoder;
  decoder->input = input;
  decoder->output = output;
  decoder->maximum = maximum;
  decoder->number_of_channels = 0;
  decoder->sample_rate = 0;
  decoder->block_align = 0;
  decoder->bits_per_sample = 0;
  decoder->did_header = 0;
  return decoder;
}

void process(struct Decoder* decoder, int offset, int length) {
  unsigned char* input = decoder->input;

  // Process header
  if (decoder->did_header == 0) {
    decoder->format = util_read_int16(input, 20);
    decoder->number_of_channels = util_read_int16(input, 22);
    decoder->sample_rate = util_read_int32(input, 24);
    decoder->block_align = util_read_int16(input, 32);
    decoder->bits_per_sample = util_read_int16(input, 34);
    set_params(decoder->number_of_channels, decoder->sample_rate);
    decoder->did_header = 1;
    offset = 44;
  }

  // Process contents
  switch (decoder->format) {
      case WAVE_FORMAT_PCM: process_pcm(decoder, offset, length); break;
      case WAVE_FORMAT_FLOAT: process_float(decoder, offset, length); break;
      case WAVE_FORMAT_ALAW: process_alaw(decoder, offset, length); break;
      case WAVE_FORMAT_MULAW: process_mulaw(decoder, offset, length); break;
      case WAVE_FORMAT_EXT: process_ext(decoder, offset, length); break;
  }
}

void process_pcm(struct Decoder* decoder, int offset, int length) {
  unsigned char* input = decoder->input;
  double* output = decoder->output;
  int const bits_per_sample = decoder->bits_per_sample;
  int const blockSize = decoder->maximum / decoder->number_of_channels;
  int index = 0;

  while (offset < length) {
    for (int channel = 0; channel < decoder->number_of_channels; channel++) {
      double sample;

      switch (decoder->bits_per_sample) {
        case 8: sample = ((double) input[offset]) / float_u8; break;
        case 16: sample = ((double) util_read_uint16(input, offset)) / float_s16; break;
        case 32: sample = ((double) util_read_int32(input, offset)) * float_s32; break;
      }

      // debug(channel, sample);
      output[(channel * blockSize) + index] = sample;
      index += 1;
    }
    offset += decoder->block_align;
  }
}

void process_float(struct Decoder* decoder, int offset, int length) {
}

void process_alaw(struct Decoder* decoder, int offset, int length) {
}

void process_mulaw(struct Decoder* decoder, int offset, int length) {
}

void process_ext(struct Decoder* decoder, int offset, int length) {
}
