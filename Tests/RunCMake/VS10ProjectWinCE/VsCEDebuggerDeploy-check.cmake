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
set(FoundEnableRedirectPlatform FALSE)
set(FoundRedirectPlatformValue FALSE)


file(STRINGS "${vcProjectFile}" lines)
foreach(line IN LISTS lines)
  if(line MATCHES "^ *<CEAdditionalFiles> *foo\\.dll\\|\\\\foo\\\\src\\\\dir\\\\on\\\\host\\|\\$\\(RemoteDirectory\\)\\|0;bar\\.dll\\|\\\\bar\\\\src\\\\dir\\|\\$\\(RemoteDirectory\\)bardir\\|0.*</CEAdditionalFiles> *$")
    set(FoundCEAdditionalFiles TRUE)
  elseif(line MATCHES " *<RemoteDirectory>[A-Za-z0-9\\]+</RemoteDirectory> *$")
    set(FoundRemoteDirectory TRUE)
  elseif(line MATCHES " *<Project +.*ToolsVersion=\"4.0\".*> *$")
    set(FoundToolsVersion4 TRUE)
  elseif(line MATCHES "^ *<EnableRedirectPlatform>true</EnableRedirectPlatform> *$")
    set(FoundEnableRedirectPlatform TRUE)
  elseif(line MATCHES "^ *<RedirectPlatformValue>.+</RedirectPlatformValue> *$")
    set(FoundRedirectPlatformValue TRUE)
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

if(NOT FoundEnableRedirectPlatform)
  set(RunCMake_TEST_FAILED "Failed to find EnableRedirectPlatform true property.")
  return()
endif()

if(NOT FoundRedirectPlatformValue)
  set(RunCMake_TEST_FAILED "Failed to find RedirectPlatformValue property.")
  return()
endif()

#
# Test solution file deployment items.
#

set(vcSlnFile "${RunCMake_TEST_BINARY_DIR}/VsCEDebuggerDeploy.sln")
if(NOT EXISTS "${vcSlnFile}")
  set(RunCMake_TEST_FAILED "Solution file ${vcSlnFile} does not exist.")
  return()
endif()


if( NOT ${CMAKE_SYSTEM_NAME} STREQUAL "WindowsCE" )
  set(RunCMake_TEST_FAILED "Test only valid for WindowsCE")
  return()
endif()


set(FooProjGUID "")
set(FoundFooProj FALSE)
set(InFooProj FALSE)
set(FoundReleaseDeploy FALSE)
set(DeployConfigs Debug MinSizeRel RelWithDebInfo )

file(STRINGS "${vcSlnFile}" lines)
foreach(line IN LISTS lines)
#message(STATUS "${line}")
  if( (NOT InFooProj ) AND (line MATCHES "^[ \\t]*Project\\(\"{[A-F0-9-]+}\"\\) = \"foo\", \"foo.vcxproj\", \"({[A-F0-9-]+})\"[ \\t]*$"))
    # First, identify the GUID for the foo project, and record it.
    set(FoundFooProj TRUE)
    set(InFooProj TRUE)
    set(FooProjGUID ${CMAKE_MATCH_1})
  elseif(InFooProj AND line MATCHES "EndProject")
    set(InFooProj FALSE)
  elseif((NOT InFooProj) AND line MATCHES "${FooProjGUID}\\.Release.*\\.Deploy\\.0")
    # If foo's Release configuration is set to deploy, this is the error.
    set(FoundReleaseDeploy TRUE)
  endif()
  if( line MATCHES "{[A-F0-9-]+}\\.([^\\|]+).*\\.Deploy\\.0" )
    # Check that the other configurations ARE set to deploy.
    list( REMOVE_ITEM DeployConfigs ${CMAKE_MATCH_1})
  endif()
endforeach()

if(FoundReleaseDeploy)
  set(RunCMake_TEST_FAILED "Release deployment not inhibited by VS_NO_SOLUTION_DEPLOY_Release.")
  return()
endif()

if(NOT FoundFooProj)
  set(RunCMake_TEST_FAILED "Failed to find foo project in the solution.")
  return()
endif()

list(LENGTH DeployConfigs length)
if(  length GREATER 0 )
  set(RunCMake_TEST_FAILED "Failed to find Deploy lines for non-Release configurations. (${length})")
  return()
endif()
