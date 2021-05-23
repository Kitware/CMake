file(READ "${RunCMake_TEST_BINARY_DIR}/internal.vcxproj" all_build)

string(REGEX MATCH
  "<ProjectReference.Include=.external.csproj.>.*</ProjectReference>"
  ProjectReference
  ${all_build}
)

if(ProjectReference STREQUAL "")
  set(RunCMake_TEST_FAILED "${test} is being set unexpectedly.")
else()
  string(REGEX MATCH
    "<ReferenceOutputAssembly>.*</ReferenceOutputAssembly>"
    ReferenceOutputAssembly
    ${ProjectReference}
  )

  if(NOT ReferenceOutputAssembly STREQUAL "")
    string(REPLACE
      "<ReferenceOutputAssembly>"
      ""
      ReferenceOutputAssemblyValue
      ${ReferenceOutputAssembly}
    )
    string(REPLACE
      "</ReferenceOutputAssembly>"
      ""
      ReferenceOutputAssemblyValue
      ${ReferenceOutputAssemblyValue}
    )

    if(ReferenceOutputAssemblyValue MATCHES "[Fa][Ll][Ss][Ee]")
      set(RunCMake_TEST_FAILED "Referenced C# project with ReferenceOutputAssembly set to false.")
    endif()
  endif()
endif()
