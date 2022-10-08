// Copyright 2022 Gareth Cross
#include <fmt/format.h>

#include <algorithm>
#include <chrono>
#include <codecvt>
#include <filesystem>
#include <locale>
#include <numeric>
#include <random>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)  //  snprintf
#define STBIW_WINDOWS_UTF8
#endif  // _MSC_VER
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#ifdef _MSC_VER
#undef STBIW_WINDOWS_UTF8
#pragma warning(pop)
#endif  // _MSC_VER

// Header w/ implementation
#include "gradient_noise/gradient_noise_internal.h"

// Generated ISPC header:
#include "gradient_noise_ispc.h"

// C++ GLM implementation of the same algorithm, for comparison:
void gradient_noise_glm(
    // Gradient noise inputs
    const unsigned int permutation_table[], const unsigned int permutation_axis_len, const float gradients[],
    // Noise parameters:
    const unsigned int octaves,
    // Output specification:
    const unsigned int width, const unsigned int height, const unsigned int output_stride, float output[]) {
  const float dx = 1.0f / static_cast<float>(width);
  const float dy = 1.0f / static_cast<float>(height);
  for (unsigned int j = 0; j < height; ++j) {
    for (unsigned int i = 0; i < width; ++i) {
      const float x = static_cast<float>(i) * dx;
      const float y = static_cast<float>(j) * dy;
      output[j * output_stride + i] =
          evaluate_noise_octaves(x, y, 0.0f, permutation_table, permutation_axis_len, gradients, octaves);
    }
  }
}

void NormalizeAndSave(const std::vector<float>& data, const unsigned int dimension, const std::string& path) {
  // Normalize the output data:
  const float min_output = *std::min_element(data.begin(), data.end());
  const float max_output = *std::max_element(data.begin(), data.end());

  std::vector<uint8_t> normalized_output;
  normalized_output.resize(dimension * dimension);
  const float scale = (max_output > min_output) ? 255.0f / (max_output - min_output) : 0.f;
  for (std::size_t i = 0; i < normalized_output.size(); ++i) {
    normalized_output[i] = static_cast<uint8_t>((data[i] - min_output) * scale);
  }

  // Save the output data
  const int success_flag = stbi_write_png(path.c_str(), static_cast<int>(dimension), static_cast<int>(dimension), 1,
                                          &normalized_output[0], static_cast<int>(dimension));
  fmt::print("{} writing png: {}\n", success_flag ? "Succeeded" : "Failed", path);
}

int main(int, char**) {
  constexpr uint32_t seed = 0;
  std::default_random_engine engine{seed};

  // Create the permutation table.
  const std::size_t perm_axis_len = 256;
  std::vector<unsigned int> permutation_table;
  permutation_table.resize(perm_axis_len * 3);
  for (std::size_t axis = 0; axis < 3; ++axis) {
    // The table is three tables: one per axis, each of length perm_axis_len.
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
#if 0
    // print the gradient vector
    fmt::print("{:.8f}, {:.8f}, {:.8f},\n", gradient_table[bin * 3 + 0], gradient_table[bin * 3 + 1], gradient_table[bin * 3 + 2]);
#endif
  }

  // Create space for output:
  const uint32_t width = 512;
  const uint32_t height = 512;

  // # of octaves to synthesize:
  const uint32_t octaves = 8;

  fmt::print("Generating gradient noise: dimension = {}, octaves = {}\n", width, octaves);

  // Run ispc kernel
  {
    std::vector<float> output;
    output.resize(width * height);

    const auto start = std::chrono::steady_clock::now();
    ispc::gradient_noise_ispc(&permutation_table[0], static_cast<uint32_t>(perm_axis_len), &gradient_table[0], octaves,
                              width, height, width, &output[0]);
    const auto finish = std::chrono::steady_clock::now();
    fmt::print("Took {:.6f} seconds (ISPC).\n",
               std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count() / 1.0e9f);

    NormalizeAndSave(output, width, "output_ispc.png");
  }

  // run it w/ C++ as well:
  {
    std::vector<float> output;
    output.resize(width * height);

    const auto start = std::chrono::steady_clock::now();
    gradient_noise_glm(&permutation_table[0], static_cast<uint32_t>(perm_axis_len), &gradient_table[0], octaves, width,
                       height, width, &output[0]);
    const auto finish = std::chrono::steady_clock::now();
    fmt::print("Took {:.6f} seconds (glm).\n",
               std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count() / 1.0e9f);

    NormalizeAndSave(output, width, "output_glm.png");
  }
  return 0;
}
