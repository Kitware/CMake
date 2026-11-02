include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/UninstalledStaticLibrary.cmake)

install(SBOM baz EXPORTS baz FORMAT spdx DESTINATION .)
