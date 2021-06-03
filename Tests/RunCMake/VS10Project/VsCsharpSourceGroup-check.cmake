set(csProjFile "${RunCMake_TEST_BINARY_DIR}/VsCsharpSourceGroup.csproj")
if(NOT EXISTS "${csProjFile}")
  set(RunCMake_TEST_FAILED "Project file ${csProjFile} does not exist.")
  return()
endif()

file(STRINGS "${csProjFile}" lines)

include(${RunCMake_TEST_SOURCE_DIR}/VsCsharpSourceGroupHelpers.cmake)

set(SOURCE_GROUPS_TO_FIND
  "CSharpSourceGroup\\\\foo\\.cs"
  "CSharpSourceGroup\\\\nested\\\\baz\\.cs"
  "CSharpSourceGroup\\\\images\\\\empty\\.bmp"
  "VsCsharpSourceGroup\\.png"
  "AssemblyInfo\\.cs"
)

foreach(GROUP_NAME IN LISTS SOURCE_GROUPS_TO_FIND)
  find_source_group("${lines}" ${GROUP_NAME})
  if(NOT ${SOURCE_GROUP_FOUND})
    return()
  endif()
endforeach()
