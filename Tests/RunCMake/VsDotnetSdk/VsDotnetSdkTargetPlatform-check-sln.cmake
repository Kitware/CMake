set(slnFile ${RunCMake_TEST_BINARY_DIR}/VsDotnetSdkTargetPlatform.sln)

if(NOT EXISTS "${slnFile}")
  string(APPEND RunCMake_TEST_FAILED
    "Solution file:\n"
    " ${slnFile}\n"
    "does not exist."
  )
  return()
endif()

file(STRINGS "${slnFile}" lines)

set(haveAnyCPU 0)
foreach(line IN LISTS lines)
  if(line MATCHES [[\.(ActiveCfg|Build\.0) = (Debug|Release|MinSizeRel|RelWithDebInfo)\|Any CPU]])
    set(haveAnyCPU 1)
  endif()
endforeach()

if(haveAnyCPU)
  string(APPEND RunCMake_TEST_FAILED
    "Solution file:\n"
    " ${slnFile}\n"
    "incorrectly maps to Any CPU."
  )
endif()
