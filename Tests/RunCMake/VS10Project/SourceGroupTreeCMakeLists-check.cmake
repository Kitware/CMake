cmake_policy(SET CMP0011 NEW)

set(vcFiltersFile "${RunCMake_TEST_BINARY_DIR}/SourceGroupTree.vcxproj.filters")
if(NOT EXISTS "${vcFiltersFile}")
  set(RunCMake_TEST_FAILED "Filters file ${vcFiltersFile} does not exist.")
  return()
endif()

file(STRINGS "${vcFiltersFile}" lines)

include(${RunCMake_TEST_SOURCE_DIR}/SourceGroupHelpers.cmake)

set(SOURCE_GROUPS_TO_FIND
  "Dir"
  "Dir\\DirNested"
  "Generated"
  "SourcesPrefix"
  "SourcesPrefix\\PrefixedNested"
)

foreach(GROUP_NAME IN LISTS ${SOURCE_GROUPS_TO_FIND})
  find_source_group("${lines}" ${GROUP_NAME})
  if(NOT ${FILTER_FOUND})
    return()
  endif()
endforeach()
