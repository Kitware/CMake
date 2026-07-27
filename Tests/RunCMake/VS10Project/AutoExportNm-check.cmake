set(vcProjectFile "${RunCMake_TEST_BINARY_DIR}/AutoExportNm.vcxproj")
if(NOT EXISTS "${vcProjectFile}")
  set(RunCMake_TEST_FAILED
    "Project file ${vcProjectFile} does not exist.")
  return()
endif()

file(READ "${vcProjectFile}" vcProject)
file(READ "${RunCMake_TEST_BINARY_DIR}/platform-toolset.txt"
  platformToolset)
if(platformToolset MATCHES [[^[Ll][Ll][Vv][Mm](_v[0-9]+(_xp)?)?$]] OR
    platformToolset MATCHES [[^[Cc][Ll][Aa][Nn][Gg]([Cc][Ll]$|_[0-9])]])
  if(NOT vcProject MATCHES
      [[__create_def[^<]*--nm=C:/cmake-nm-test/tool-nm\.exe]])
    set(RunCMake_TEST_FAILED
      "__create_def does not pass CMAKE_NM in ${vcProjectFile}.")
  endif()
else()
  if(vcProject MATCHES
      [[__create_def[^<]*--nm=C:/cmake-nm-test/tool-nm\.exe]])
    set(RunCMake_TEST_FAILED
      "__create_def passes CMAKE_NM for toolset ${platformToolset}.")
  endif()
endif()
