# Fake out that we have dyndep; we only need to generate, not actually build
# here.
set(CMAKE_CXX_SCANDEP_SOURCE "")

enable_language(CXX)

if (NOT CMAKE_GENERATOR MATCHES "Ninja")
  message(FATAL_ERROR
    "This test requires a 'Ninja' generator to be used.")
endif ()

add_library(ninja-exports-public)
target_sources(ninja-exports-public
  PRIVATE
    sources/module-impl.cxx
    sources/module-internal-part-impl.cxx
    sources/module-part-impl.cxx
    sources/module-use.cxx
  PUBLIC
    FILE_SET modules TYPE CXX_MODULES
    BASE_DIRS
      "${CMAKE_CURRENT_SOURCE_DIR}/sources"
    FILES
      sources/module.cxx
      sources/module-part.cxx
    FILE_SET internal_partitions TYPE CXX_MODULES FILES
      sources/module-internal-part.cxx)
target_compile_features(ninja-exports-public
  PRIVATE
    cxx_std_20)
set_property(TARGET ninja-exports-public
  PROPERTY EXPORT_NAME "with-public")

install(TARGETS ninja-exports-public
  EXPORT exp
  FILE_SET modules
    DESTINATION "lib/cxx"
    COMPONENT "modules"
  FILE_SET internal_partitions
    DESTINATION "lib/cxx/internals"
    COMPONENT "modules-internal")

add_library(ninja-exports-private)
target_sources(ninja-exports-private
  PRIVATE
    sources/module-impl.cxx
    sources/module-internal-part-impl.cxx
    sources/module-part-impl.cxx
    sources/module-use.cxx
  PRIVATE
    FILE_SET modules TYPE CXX_MODULES
    BASE_DIRS
      "${CMAKE_CURRENT_SOURCE_DIR}/sources"
    FILES
      sources/module.cxx
      sources/module-part.cxx
    FILE_SET internal_partitions TYPE CXX_MODULES FILES
      sources/module-internal-part.cxx)
target_compile_features(ninja-exports-private
  PRIVATE
    cxx_std_20)
set_property(TARGET ninja-exports-private
  PROPERTY EXPORT_NAME "with-private")

install(TARGETS ninja-exports-private
  EXPORT exp)

# Test multiple build exports.
export(EXPORT exp
  FILE "${CMAKE_BINARY_DIR}/lib/cmake/export1/export1-targets.cmake"
  NAMESPACE export1::
  CXX_MODULES_DIRECTORY "cxx-modules")
export(EXPORT exp
  FILE "${CMAKE_BINARY_DIR}/lib/cmake/export2/export2-targets.cmake"
  CXX_MODULES_DIRECTORY "cxx-modules")

# Test multiple install exports.
install(EXPORT exp
  DESTINATION "lib/cmake/export1"
  NAMESPACE export1::
  CXX_MODULES_DIRECTORY "cxx-modules")
install(EXPORT exp
  DESTINATION "lib/cmake/export2"
  CXX_MODULES_DIRECTORY "cxx-modules")
