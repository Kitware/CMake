include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/Setup.cmake)


include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

add_library(interface INTERFACE)

target_include_directories(interface INTERFACE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

install(
  TARGETS interface
  EXPORT interface_targets
)

install(SBOM interface_targets
  EXPORT interface_targets
  DESTINATION .
)
