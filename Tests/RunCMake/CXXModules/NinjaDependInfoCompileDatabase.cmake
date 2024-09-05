# Fake out that we have dyndep; we only need to generate, not actually build
# here.
set(CMAKE_CXX_SCANDEP_SOURCE "")

set(CMAKE_EXPERIMENTAL_EXPORT_BUILD_DATABASE "4bd552e2-b7fb-429a-ab23-c83ef53f3f13")

enable_language(CXX)

if (NOT CMAKE_GENERATOR MATCHES "Ninja")
  message(FATAL_ERROR
    "This test requires a 'Ninja' generator to be used.")
endif ()

add_library(ninja-compiledb-public)
target_sources(ninja-compiledb-public
  PRIVATE
    sources/module-impl.cxx
    sources/module-internal-part-impl.cxx
    sources/module-part-impl.cxx
  PUBLIC
    FILE_SET modules TYPE CXX_MODULES
    BASE_DIRS
      "${CMAKE_CURRENT_SOURCE_DIR}/sources"
    FILES
      sources/module.cxx
      sources/module-part.cxx
    FILE_SET internal_partitions TYPE CXX_MODULES FILES
      sources/module-internal-part.cxx)
target_compile_features(ninja-compiledb-public
  PRIVATE
    cxx_std_20)
set_property(TARGET ninja-compiledb-public
  PROPERTY EXPORT_BUILD_DATABASE 1)

add_library(ninja-compiledb-private)
target_sources(ninja-compiledb-private
  PRIVATE
    sources/module-impl.cxx
    sources/module-internal-part-impl.cxx
    sources/module-part-impl.cxx
  PRIVATE
    FILE_SET modules TYPE CXX_MODULES
    BASE_DIRS
      "${CMAKE_CURRENT_SOURCE_DIR}/sources"
    FILES
      sources/module.cxx
      sources/module-part.cxx
    FILE_SET internal_partitions TYPE CXX_MODULES FILES
      sources/module-internal-part.cxx)
target_compile_features(ninja-compiledb-private
  PRIVATE
    cxx_std_20)
set_property(TARGET ninja-compiledb-private
  PROPERTY EXPORT_BUILD_DATABASE 1)
