set(vcProjectFile "${RunCMake_TEST_BINARY_DIR}/foo.vcxproj")
if(NOT EXISTS "${vcProjectFile}")
  set(RunCMake_TEST_FAILED "Project file ${vcProjectFile} does not exist.")
  return()
endif()


if( NOT ${CMAKE_SYSTEM_NAME} STREQUAL "WindowsCE" )
  set(RunCMake_TEST_FAILED "Test only valid for WindowsCE")
  return()
endif()


set(FoundCEAdditionalFiles FALSE)
set(FoundRemoteDirectory FALSE)
set(FoundToolsVersion4 FALSE)

file(STRINGS "${vcProjectFile}" lines)
foreach(line IN LISTS lines)
  if(line MATCHES "^ *<CEAdditionalFiles> *foo\\.dll\\|\\\\foo\\\\src\\\\dir\\\\on\\\\host\\|\\$\\(RemoteDirectory\\)\\|0;bar\\.dll\\|\\\\bar\\\\src\\\\dir\\|\\$\\(RemoteDirectory\\)bardir\\|0.*</CEAdditionalFiles> *$")
    set(FoundCEAdditionalFiles TRUE)
  elseif(line MATCHES " *<RemoteDirectory>[A-Za-z0-9\\]+</RemoteDirectory> *$")
    set(FoundRemoteDirectory TRUE)
  elseif(line MATCHES " *<Project +.*ToolsVersion=\"4.0\".*> *$")
    set(FoundToolsVersion4 TRUE)
  endif()
endforeach()

if(NOT FoundCEAdditionalFiles)
  set(RunCMake_TEST_FAILED "CEAddionalFiles not found or not set correctly.")
  return()
endif()

if(NOT FoundRemoteDirectory)
  set(RunCMake_TEST_FAILED "RemoteDirectory not found or not set correctly.")
  return()
endif()

if(NOT FoundToolsVersion4)
  set(RunCMake_TEST_FAILED "Failed to find correct ToolsVersion=\"4.0\" .")
  return()
endif()
