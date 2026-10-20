foreach(sbom IN ITEMS bundle selected unmatched)
  file(READ "${RunCMake_TEST_INSTALL_DIR}/${sbom}-Debug.spdx.json" content)
  include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ProjectLicense-install-check.cmake)
endforeach()
