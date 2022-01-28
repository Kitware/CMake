project(zstd C)

# Disable warnings to avoid changing 3rd party code.
if(CMAKE_C_COMPILER_ID MATCHES
    "^(GNU|LCC|Clang|AppleClang|IBMClang|XLClang|XL|VisualAge|SunPro|HP|Intel|IntelLLVM|NVHPC)$")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
elseif(CMAKE_C_COMPILER_ID STREQUAL "PathScale")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -woffall")
endif()

include_directories(lib lib/common)

add_library(cmzstd STATIC
  lib/common/entropy_common.c
  lib/common/error_private.c
  lib/common/fse_decompress.c
  lib/common/pool.c
  lib/common/threading.c
  lib/common/xxhash.c
  lib/common/zstd_common.c
  lib/compress/fse_compress.c
  lib/compress/hist.c
  lib/compress/huf_compress.c
  lib/compress/zstd_compress_literals.c
  lib/compress/zstd_compress.c
  lib/compress/zstd_compress_sequences.c
  lib/compress/zstd_compress_superblock.c
  lib/compress/zstd_double_fast.c
  lib/compress/zstd_fast.c
  lib/compress/zstd_lazy.c
  lib/compress/zstd_ldm.c
  lib/compress/zstdmt_compress.c
  lib/compress/zstd_opt.c
  lib/decompress/huf_decompress.c
  lib/decompress/zstd_ddict.c
  lib/decompress/zstd_decompress_block.c
  lib/decompress/zstd_decompress.c
  lib/deprecated/zbuff_common.c
  lib/deprecated/zbuff_compress.c
  lib/deprecated/zbuff_decompress.c
  lib/dictBuilder/cover.c
  lib/dictBuilder/divsufsort.c
  lib/dictBuilder/fastcover.c
  lib/dictBuilder/zdict.c
  )

# BMI2 instructions are not supported in older environments.
set_property(TARGET cmzstd PROPERTY COMPILE_DEFINITIONS DYNAMIC_BMI2=0)

install(FILES LICENSE DESTINATION ${CMAKE_DOC_DIR}/cmzstd)
