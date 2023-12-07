# Fake out that we have dyndep; we only need to generate, not actually build
# here.
set(CMAKE_CXX_SCANDEP_SOURCE "")

enable_language(CXX)

if (NOT CMAKE_GENERATOR MATCHES "Ninja")
  message(FATAL_ERROR
    "This test requires a 'Ninja' generator to be used.")
endif ()

add_library(ninja-file-sets-public)
target_sources(ninja-file-sets-public
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
target_compile_features(ninja-file-sets-public
  PRIVATE
    cxx_std_20)

install(TARGETS ninja-file-sets-public
  FILE_SET modules
    DESTINATION "lib/cxx"
    COMPONENT "modules"
  FILE_SET internal_partitions
    DESTINATION "lib/cxx/internals"
    COMPONENT "modules-internal")

add_library(ninja-file-sets-private)
target_sources(ninja-file-sets-private
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
target_compile_features(ninja-file-sets-private
  PRIVATE
    cxx_std_20)
