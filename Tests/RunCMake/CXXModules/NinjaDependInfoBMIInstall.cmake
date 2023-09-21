# Fake out that we have dyndep; we only need to generate, not actually build
# here.
set(CMAKE_CXX_SCANDEP_SOURCE "")

enable_language(CXX)

if (NOT CMAKE_GENERATOR MATCHES "Ninja")
  message(FATAL_ERROR
    "This test requires a 'Ninja' generator to be used.")
endif ()

add_library(ninja-bmi-install-public)
target_sources(ninja-bmi-install-public
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
target_compile_features(ninja-bmi-install-public
  PRIVATE
    cxx_std_20)
set_property(TARGET ninja-bmi-install-public
  PROPERTY EXPORT_NAME "with-public")

install(TARGETS ninja-bmi-install-public
  FILE_SET modules
    DESTINATION "lib/cxx"
    COMPONENT "modules"
  FILE_SET internal_partitions
    DESTINATION "lib/cxx/internals"
    COMPONENT "modules-internal"
  CXX_MODULES_BMI
    DESTINATION "lib/cxx/modules/$<CONFIG>"
    COMPONENT "bmi")

add_library(ninja-bmi-install-private)
target_sources(ninja-bmi-install-private
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
target_compile_features(ninja-bmi-install-private
  PRIVATE
    cxx_std_20)
set_property(TARGET ninja-bmi-install-private
  PROPERTY EXPORT_NAME "with-private")

set(CMAKE_INSTALL_MESSAGE LAZY)
install(TARGETS ninja-bmi-install-private
  CXX_MODULES_BMI
    DESTINATION "lib/cxx/modules/private/$<CONFIG>"
    PERMISSIONS
      OWNER_READ OWNER_WRITE
      GROUP_READ
      WORLD_READ
    COMPONENT "bmi")
