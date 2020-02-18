set(vcProjectFile "${RunCMake_TEST_BINARY_DIR}/foo.vcxproj")
if(NOT EXISTS "${vcProjectFile}")
  set(RunCMake_TEST_FAILED "Project file ${vcProjectFile} does not exist.")
  return()
endif()
#
# Test solution file for deployment.
#

set(vcSlnFile "${RunCMake_TEST_BINARY_DIR}/VsDeployEnabled.sln")
if(NOT EXISTS "${vcSlnFile}")
  set(RunCMake_TEST_FAILED "Solution file ${vcSlnFile} does not exist.")
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
  set(RunCMake_TEST_FAILED "Release deployment enabled.")
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
