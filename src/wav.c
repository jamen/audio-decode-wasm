
#include <stdint.h>
#include <endian.h>

#define util_read_int32(i,o) (i[o]|(i[o+1]<<8)|(i[o+2]<<16)|(i[o+3]<<24))
#define util_read_int16(i,o) (i[o]|(i[o+1]<<8))

#define WAVE_FORMAT_PCM 0x0001
#define WAVE_FORMAT_FLOAT 0x0003
#define WAVE_FORMAT_ALAW 0x0006
#define WAVE_FORMAT_MULAW 0x0007
#define WAVE_FORMAT_EXT 0xFFFE

#define lossy_32_24_float 1.0f / (32768.0f * 65536.0f)

extern void debug(int);
extern unsigned char set_params(int, int);

typedef struct {
  unsigned char* input;
  float* output;
  int maximum;
  int format;
  int number_of_channels;
  int sample_rate;
  int bits_per_sample;
  int block_align;
  unsigned char did_header;
} Context;

Context* open(unsigned char* input, float* output, int length);
void process(Context* context, int offset, const int length);
void process_pcm(Context* context, int* offset, const int length);
void process_float(Context* context, int* offset, const int length);
void process_alaw(Context* context, int* offset, const int length);
void process_mulaw(Context* context, int* offset, const int length);
void process_ext(Context* context, int* offset, const int length);

Context* open(unsigned char* input, float* output, int maximum) {
  Context* context;
  context->input = input;
  context->output = output;
  context->maximum = maximum;
  context->number_of_channels = 0;
  context->sample_rate = 0;
  context->block_align = 0;
  context->bits_per_sample = 0;
  context->did_header = 0;
  return context;
}

void process(Context* context, int offset, const int length) {
  unsigned char* input = context->input;

  // Process header
  if (context->did_header == 0) {
    context->format = util_read_int16(input, 20);
    context->number_of_channels = util_read_int16(input, 22);
    context->sample_rate = util_read_int32(input, 24);
    context->block_align = util_read_int16(input, 32);
    context->bits_per_sample = util_read_int16(input, 34);
    set_params(context->number_of_channels, context->sample_rate);
    context->did_header = 1;
    offset = 44;
  }

  // Process contents
  switch (context->format) {
      case WAVE_FORMAT_PCM: process_pcm(context, &offset, length); break;
      case WAVE_FORMAT_FLOAT: process_float(context, &offset, length); break;
      case WAVE_FORMAT_ALAW: process_alaw(context, &offset, length); break;
      case WAVE_FORMAT_MULAW: process_mulaw(context, &offset, length); break;
      case WAVE_FORMAT_EXT: process_ext(context, &offset, length); break;
  }
}

void process_pcm(Context* context, int* offset, const int length) {
  unsigned char* input = context->input;
  float* output = context->output;
  int const bits_per_sample = context->bits_per_sample;
  int const block = context->maximum / context->number_of_channels;
  int index = 0;

  while (*offset < length) {
    for (int channel = 0; channel < context->number_of_channels; channel++) {
      switch (context->bits_per_sample) {
        case 8: output[(channel * block) + index] = ((float) input[*offset]) / 256.0f; break;
        case 16: output[(channel * block) + index] = ((float) util_read_int16(input, *offset)) / 65536.0f; break;
        case 32: output[(channel * block) + index] = (float) (util_read_int32(input, *offset) * lossy_32_24_float); break;
      }
    }

    *offset += context->block_align;
    index += 1;
  }
}

void process_float(Context* context, int* offset, const int length) {
}

void process_alaw(Context* context, int* offset, const int length) {
}

void process_mulaw(Context* context, int* offset, const int length) {
}

void process_ext(Context* context, int* offset, const int length) {
}
