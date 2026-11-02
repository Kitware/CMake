include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/IgnoresInterfaceDirs.cmake)

install(SBOM interface_targets
  EXPORTS interface_targets
  DESTINATION .
)
