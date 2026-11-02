foreach(sbom IN ITEMS bundle selected unmatched)
  file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/${sbom}/${sbom}-Debug.spdx.json" content)
  include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ProjectLicense-install-check.cmake)
endforeach()
