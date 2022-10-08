// Copyright 2022 Gareth Cross
#include "gradient_noise_internal.h"

export void gradient_noise_ispc(
    // Gradient noise inputs
    const uniform unsigned int permutation_table[], const uniform unsigned int permutation_axis_len,
    const uniform float gradients[],
    // Noise parameters:
    const uniform unsigned int octaves,
    // Output specification:
    const uniform unsigned int width, const uniform unsigned int height, const uniform unsigned int output_stride,
    uniform float output[]) {

  const uniform float dx = 1.0f / (float)width;
  const uniform float dy = 1.0f / (float)height;
  for (uniform int j = 0; j < height; j++) {
    for (uniform int i = 0; i < width; i += programCount) {
      const float x = (i + programIndex) * dx;
      const float y = j * dy;
      output[j * output_stride + i + programIndex] =
          evaluate_noise_octaves(x, y, 0.0f, permutation_table, permutation_axis_len, gradients, octaves);
    }
  }
}
