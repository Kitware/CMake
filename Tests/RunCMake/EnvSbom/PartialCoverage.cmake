project(test_project)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/Setup.cmake)

add_library(libA INTERFACE)
add_library(libB INTERFACE)

install(TARGETS libA EXPORT explicit_root DESTINATION .)
install(TARGETS libB EXPORT implicit_root DESTINATION .)

install(SBOM explicit_root_sbom EXPORTS explicit_root DESTINATION .)

add_subdirectory(PartialCoverage_subdir)
