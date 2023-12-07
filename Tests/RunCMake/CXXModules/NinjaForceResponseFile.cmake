# Fake out that we have dyndep; we only need to generate, not actually build
# here.
set(CMAKE_CXX_SCANDEP_SOURCE "")

enable_language(CXX)

if (NOT CMAKE_GENERATOR MATCHES "Ninja")
  message(FATAL_ERROR
    "This test requires a 'Ninja' generator to be used.")
endif ()

set(CMAKE_NINJA_FORCE_RESPONSE_FILE 1)

add_library(ninja-forced-response-file)
target_sources(ninja-forced-response-file
  PRIVATE
    FILE_SET modules TYPE CXX_MODULES
    BASE_DIRS
      "${CMAKE_CURRENT_SOURCE_DIR}/sources"
    FILES
      sources/module.cxx
      sources/module-part.cxx
    FILE_SET internal_partitions TYPE CXX_MODULES FILES
      sources/module-internal-part.cxx)
target_compile_features(ninja-forced-response-file
  PRIVATE
    cxx_std_20)
