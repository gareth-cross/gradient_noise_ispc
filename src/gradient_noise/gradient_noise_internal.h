// Copyright 2022 Gareth Cross
#pragma once

#if defined(__cplusplus)
// Compling as C++
#define uniform

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)  //  nonstandard extension: nameless struct/union
#endif                           // _MSC_VER
#define GLM_FORCE_INLINE
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif  // _MSC_VER

// Comment out to use the version that computes fibonacci sphere in gradient.
#define SAMPLE_PRECOMPUTED_GRADIENTS

// Alias to GLM type:
typedef glm::vec3 float3;
#define make_float3(x, y, z) \
  glm::vec3 { (x), (y), (z) }

inline float dot(const float3& a, const float3& b) { return glm::dot(a, b); }

// Need min() in C++
#include <utility>
#define min(a, b) std::min((a), (b))
#else
// Compiling as ISPC
typedef float<3> float3;
#define make_float3(x, y, z) \
  { (x), (y), (z) }

inline float dot(const float3 a, const float3 b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }
#endif  // __cplusplus

inline unsigned int noise_hash(const int x, const int y, const int z, const uniform unsigned int permutation_table[],
                               const uniform unsigned int permutation_axis_len) {
  const uniform unsigned int permutation_mask = permutation_axis_len - 1;
  return permutation_table[(unsigned int)x & permutation_mask] ^
         permutation_table[(unsigned int)y & permutation_mask + permutation_axis_len] ^
         permutation_table[(unsigned int)z & permutation_mask + permutation_axis_len * 2];
}

#if defined(__cplusplus)
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)  // Unreferenced formal parameter.
#endif  // _MSC_VER
#endif  // __cplusplus

inline float3 gradient(const int x, const int y, const int z, const uniform unsigned int permutation_table[],
                       const uniform unsigned int permutation_axis_len, const uniform float gradients[]) {
  const unsigned int hash = noise_hash(x, y, z, permutation_table, permutation_axis_len);
#if defined(SAMPLE_PRECOMPUTED_GRADIENTS)
  // Sample gradient from the provided table.
  const unsigned int index = hash * 3;
  const float3 result = make_float3(gradients[index + 0], gradients[index + 1], gradients[index + 2]);
#else
  // Take the hashed integer, and use it an index into a fibonacci sphere.
  // ISPC absolutely dominates this version. The speed increase is > 10x from non-simd version.
  // Overall though, the table method is still faster.
  const float k = hash + .5f;
  const float cos_phi = 1.0f - 2.0f * k / (float)permutation_axis_len;  // bounds = (-1, 1)
  const float sin_phi = sqrt(1.0f - cos_phi * cos_phi);                 // bounds = (0, 1)
  const float theta = 3.14159265359f * (1 + sqrt(5.0f)) * k;
  const float3 result = make_float3(cos(theta) * sin_phi, sin(theta) * sin_phi, cos_phi);
#endif  // SAMPLE_PRECOMPUTED_GRADIENTS
  return result;
}

#if defined(__cplusplus)
#ifdef _MSC_VER
#pragma warning(pop)
#endif  // _MSC_VER
#endif  // __cplusplus

inline float lerp(const float a, const float b, const float alpha) { return a * (1 - alpha) + b * alpha; }

inline float evaluate_noise(const float x, const float y, const float z, const uniform unsigned int permutation_table[],
                            const uniform unsigned int permutation_axis_len, const uniform float gradients[]) {
  const int ix = (int)floor(x);
  const int iy = (int)floor(y);
  const int iz = (int)floor(z);

  // Compute fractional part and the smoother-step polynomial:
  const float3 f = make_float3(x - ix, y - iy, z - iz);
  const float3 alpha = f * f * f * (6.0f * f * f - 15.0f * f + 10.0f);

  // Sample gradients at the corners of the the bin:
  const float3 c000 = gradient(ix, iy, iz, permutation_table, permutation_axis_len, gradients);
  const float3 c001 = gradient(ix, iy, iz + 1, permutation_table, permutation_axis_len, gradients);
  const float3 c010 = gradient(ix, iy + 1, iz, permutation_table, permutation_axis_len, gradients);
  const float3 c011 = gradient(ix, iy + 1, iz + 1, permutation_table, permutation_axis_len, gradients);
  const float3 c100 = gradient(ix + 1, iy, iz, permutation_table, permutation_axis_len, gradients);
  const float3 c101 = gradient(ix + 1, iy, iz + 1, permutation_table, permutation_axis_len, gradients);
  const float3 c110 = gradient(ix + 1, iy + 1, iz, permutation_table, permutation_axis_len, gradients);
  const float3 c111 = gradient(ix + 1, iy + 1, iz + 1, permutation_table, permutation_axis_len, gradients);
  const float3 v000 = make_float3(f[0], f[1], f[2]);
  const float3 v001 = make_float3(f[0], f[1], f[2] - 1.f);
  const float3 v010 = make_float3(f[0], f[1] - 1.f, f[2]);
  const float3 v011 = make_float3(f[0], f[1] - 1.f, f[2] - 1.f);
  const float3 v100 = make_float3(f[0] - 1.f, f[1], f[2]);
  const float3 v101 = make_float3(f[0] - 1.f, f[1], f[2] - 1.f);
  const float3 v110 = make_float3(f[0] - 1.f, f[1] - 1.f, f[2]);
  const float3 v111 = make_float3(f[0] - 1.f, f[1] - 1.f, f[2] - 1.f);
  const float p000 = dot(c000, v000);
  const float p001 = dot(c001, v001);
  const float p010 = dot(c010, v010);
  const float p011 = dot(c011, v011);
  const float p100 = dot(c100, v100);
  const float p101 = dot(c101, v101);
  const float p110 = dot(c110, v110);
  const float p111 = dot(c111, v111);
  // 3D interpolation:
  const float fx_0 = lerp(p000, p100, alpha[0]);
  const float fx_1 = lerp(p001, p101, alpha[0]);
  const float fx_2 = lerp(p010, p110, alpha[0]);
  const float fx_3 = lerp(p011, p111, alpha[0]);
  const float fy_0 = lerp(fx_0, fx_2, alpha[1]);
  const float fy_1 = lerp(fx_1, fx_3, alpha[1]);
  const float fz_0 = lerp(fy_0, fy_1, alpha[2]);
  return fz_0;
}

inline float evaluate_noise_octaves(const float x, const float y, const float z,
                                    const uniform unsigned int permutation_table[],
                                    const uniform unsigned int permutation_axis_len, const uniform float gradients[],
                                    const uniform unsigned int octaves) {
  float sum = 0.0f;
  float amplitude = 0.5f;
  float scale = 1.0f;
  for (uniform unsigned int i = 0; i < octaves; ++i) {
    sum +=
        amplitude * evaluate_noise(x * scale, y * scale, z * scale, permutation_table, permutation_axis_len, gradients);
    amplitude *= 0.5f;
    scale *= 2.0f;
  }
  return sum;
}

#if defined(__cplusplus)
#undef uniform
#endif  // __cplusplus
