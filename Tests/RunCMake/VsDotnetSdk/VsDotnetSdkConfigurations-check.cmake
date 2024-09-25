set(csProjectFile ${RunCMake_TEST_BINARY_DIR}/foo.csproj)

if(NOT EXISTS "${csProjectFile}")
  set(RunCMake_TEST_FAILED "Project file ${csProjectFile} does not exist.")
  return()
endif()

set(hasConfigurations FALSE)

file(STRINGS "${csProjectFile}" lines)

foreach(line IN LISTS lines)
  if(line MATCHES "<Configurations>Debug;Release;MinSizeRel;RelWithDebInfo;ExtraTestConfig</Configurations>")
    set(hasConfigurations TRUE)
  endif()
endforeach()

if(NOT hasConfigurations)
  set(RunCMake_TEST_FAILED "<Configurations> not found in ${csProjectFile}.")
endif()
