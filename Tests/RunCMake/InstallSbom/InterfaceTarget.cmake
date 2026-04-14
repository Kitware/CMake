include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/InterfaceTarget.cmake)

install(SBOM interface_targets
  EXPORTS interface_targets
  DESTINATION .
)
