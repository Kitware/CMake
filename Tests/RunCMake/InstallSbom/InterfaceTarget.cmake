include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/InterfaceTarget.cmake)

install(SBOM interface_targets
  EXPORT interface_targets
  DESTINATION .
)
