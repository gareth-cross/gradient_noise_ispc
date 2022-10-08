// Copyright 2022 Gareth Cross
#include <fmt/format.h>

#include <algorithm>
#include <chrono>
#include <numeric>
#include <random>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)  //  snprintf
#endif                           // _MSC_VER
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif  // _MSC_VER

// Generated ISPC header:
#include "gradient_noise_ispc.h"

int main(int, char **) {
  std::default_random_engine engine{0};

  // Create the permutation table.
  const std::size_t perm_axis_len = 256;
  std::vector<unsigned int> permutation_table;
  permutation_table.resize(perm_axis_len * 3);
  for (std::size_t axis = 0; axis < 3; ++axis) {
    const auto start = permutation_table.begin() + perm_axis_len * axis;
    std::iota(start, start + perm_axis_len, 0u);
    std::shuffle(start, start + perm_axis_len, engine);
  }

  // Create the gradient table.
  // https://mathworld.wolfram.com/SpherePointPicking.html
  const std::uniform_real_distribution<float> uniform{0.f, 1.f};
  std::vector<float> gradient_table;
  gradient_table.resize(perm_axis_len * 3);
  for (std::size_t bin = 0; bin < perm_axis_len; ++bin) {
    const float elevation = std::acos(std::clamp(2.f * uniform(engine) - 1.f, -1.f, 1.f));
    const float azimuth = 2.0f * uniform(engine) * M_PI;
    gradient_table[bin * 3 + 0] = std::cos(azimuth) * std::sin(elevation);
    gradient_table[bin * 3 + 1] = std::sin(azimuth) * std::sin(elevation);
    gradient_table[bin * 3 + 2] = std::cos(elevation);
  }

  // Create space for output:
  const uint32_t width = 512;
  const uint32_t height = 512;
  std::vector<float> output;
  output.resize(width * height);

  // Run ispc kernel
  fmt::print("Running gradient noise...\n");
  const uint32_t octaves = 8;

  const auto start = std::chrono::steady_clock::now();
  ispc::gradient_noise_ispc(&permutation_table[0], static_cast<uint32_t>(perm_axis_len), &gradient_table[0], octaves,
                            width, height, width, &output[0]);
  const auto finish = std::chrono::steady_clock::now();
  fmt::print("Took {:.6f} seconds.\n",
             std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count() / 1.0e9f);

  // Normalize the output data:
  const float min_output = *std::min_element(output.begin(), output.end());
  const float max_output = *std::max_element(output.begin(), output.end());
  fmt::print("Min/max values: {} / {}\n", min_output, max_output);

  std::vector<uint8_t> normalized_output;
  normalized_output.resize(width * height);
  const float scale = (max_output > min_output) ? 255.0f / (max_output - min_output) : 0.f;
  for (std::size_t i = 0; i < normalized_output.size(); ++i) {
    normalized_output[i] = static_cast<uint8_t>((output[i] - min_output) * scale);
  }

  // Save the output data
  const int success_flag = stbi_write_png("output.png", static_cast<int>(width), static_cast<int>(height), 1,
                                          &normalized_output[0], static_cast<int>(width));
  fmt::print("Success writing png: {}\n", success_flag);
  return success_flag == 1;
}
