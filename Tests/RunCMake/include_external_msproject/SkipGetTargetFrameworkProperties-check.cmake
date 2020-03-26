file(READ "${RunCMake_TEST_BINARY_DIR}/ALL_BUILD.vcxproj" all_build)

macro(project_reference EXTERNAL_PROJECT)
  string(REGEX MATCH
    "<ProjectReference.Include=.${${EXTERNAL_PROJECT}}.>.*</SkipGetTargetFrameworkProperties>"
    EndOfProjectReference
    ${all_build}
    )
endmacro()

set(external_project "external.project")
project_reference(external_project)
if(NOT ${EndOfProjectReference} MATCHES ".*</ProjectReference>")
  set(RunCMake_TEST_FAILED "${test} is being set unexpectedly.")
endif()

set(external_project "external.csproj")
project_reference(external_project)
if(${EndOfProjectReference} MATCHES ".*</ProjectReference>")
  set(RunCMake_TEST_FAILED "${test} is not set.")
endif()
