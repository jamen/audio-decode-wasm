
#include <stdint.h>
#include <endian.h>

#define read_int32(i,o) i[o]|(i[o+1]<<8)|(i[o+2]<<16)|(i[o+3]<<24)
#define read_int16(i,o) i[o]|(i[o+1]<<8)

typedef struct {
  unsigned char* input;
  float* output;
  uint16_t number_of_channels;
  uint32_t sample_rate;
  uint16_t format;
  uint32_t avg_byte_rate;
  uint16_t bits_per_sample;
  unsigned char params;
} Context;

extern const int CHUNK_SIZE;
extern unsigned char set_params(uint16_t, uint32_t);

Context* open(unsigned char* input, float* output) {
  Context context = {
    .input = input,
    .output = output,
    .number_of_channels = 0,
    .sample_rate = 0,
    .avg_byte_rate = 0,
    .bits_per_sample = 0,
    .params = 0
  };

  return &context;
}

void process(Context* context, int amount) {
  unsigned char* input = context->input;
  int input_step = 0;

  if (context->params == 0) {
    context->format = read_int16(input, 20);
    context->number_of_channels = read_int16(input, 22);
    context->sample_rate = read_int32(input, 24);
    context->avg_byte_rate = read_int32(input, 28);
    context->bits_per_sample = read_int16(input, 32);
    set_params(context->number_of_channels, context->sample_rate);
    context->params = 1;
    input_step = 44;
  }

  float* output = context->output;
  int number_of_channels = context->number_of_channels;
  int bits_per_sample = context->bits_per_sample;
  int length = CHUNK_SIZE / number_of_channels;
  int step = 0;

  while (input_step < amount) {
    for (int channel = 0; channel < number_of_channels; channel++) {
      int input_index = input_step + (channel * bits_per_sample);
      int output_index =  step + (channel * length);
      switch (bits_per_sample) {
        case 8:
          output[output_index] = ((float) input[input_index]) / 256.00f;
          break;
        case 16:
          output[output_index] = ((float) input[input_index]) / 65536.00f;
          break;
        case 32:
          output[output_index] = ((float) input[input_index]) / 4294967296.00f;
          break;
      }
      step += 1;
      input_step += 1;
    }
  }
}
