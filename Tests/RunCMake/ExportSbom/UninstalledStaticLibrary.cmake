include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/UninstalledStaticLibrary.cmake)

export(SBOM baz EXPORTS baz FORMAT spdx)
