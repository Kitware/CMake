set(vcFiltersFile "${RunCMake_TEST_BINARY_DIR}/foo.vcxproj.filters")
if(NOT EXISTS "${vcFiltersFile}")
  set(RunCMake_TEST_FAILED "Filters file ${vcFiltersFile} does not exist.")
  return()
endif()

file(STRINGS "${vcFiltersFile}" lines)

include(${RunCMake_TEST_SOURCE_DIR}/SourceGroupHelpers.cmake)

find_source_group("${lines}" CMakeListsSourceGroup)
