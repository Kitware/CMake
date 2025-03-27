set(csProjectFile ${RunCMake_TEST_BINARY_DIR}/foo.csproj)

if(NOT EXISTS "${csProjectFile}")
  set(RunCMake_TEST_FAILED "Project file ${csProjectFile} does not exist.")
  return()
endif()

set(hasAssemblyName FALSE)

file(STRINGS "${csProjectFile}" lines)

foreach(line IN LISTS lines)
  if(NOT inLib1)
    if(line MATCHES "<AssemblyName>longer name</AssemblyName>")
      set(hasAssemblyName TRUE)
    endif()
  endif()
endforeach()

if(NOT hasAssemblyName)
  set(RunCMake_TEST_FAILED "<AssemblyName> not found in ${csProjectFile}.")
endif()
