# Build ISPC code
set (ISPC_SRC_NAME "gradient_noise")
set (ISPC_IA_TARGETS "sse2-i32x4,sse4-i32x4,avx1-i32x16,avx2-i32x16,avx512knl-x16,avx512skx-x16" CACHE STRING "ISPC IA targets")
set (ISPC_ARM_TARGETS "neon" CACHE STRING "ISPC ARM targets")
add_ispc_library(NAME "gradient_noise"
                 ISPC_IA_TARGETS ${ISPC_IA_TARGETS}
                 ISPC_ARM_TARGETS ${ISPC_ARM_TARGETS}
                 ISPC_SRC_NAME ${ISPC_SRC_NAME}
                 USED_HEADERS "gradient_noise_internal.h"
                 USE_COMMON_SETTINGS)
