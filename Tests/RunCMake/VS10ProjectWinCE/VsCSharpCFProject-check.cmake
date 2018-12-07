#
# Check C# Compact Framework project for required elements.
#
set(csProjectFile "${RunCMake_TEST_BINARY_DIR}/foo.csproj")
if(NOT EXISTS "${csProjectFile}")
  set(RunCMake_TEST_FAILED "Project file ${csProjectFile} does not exist.")
  return()
endif()

if( NOT ${CMAKE_SYSTEM_NAME} STREQUAL "WindowsCE" )
  set(RunCMake_TEST_FAILED "Test only valid for WindowsCE")
  return()
endif()

set(FoundTargetFrameworkTargetsVersion FALSE)
set(FoundDotNetFrameworkVersion FALSE)
set(FoundTargetFrameworkIdentifier FALSE)
set(FoundCFTargetsImport FALSE)


file(STRINGS "${csProjectFile}" lines)
foreach(line IN LISTS lines)
  #message(STATUS ${line})
  if(line MATCHES "^ *<TargetFrameworkIdentifier>WindowsEmbeddedCompact</TargetFrameworkIdentifier> *$")
    set(FoundTargetFrameworkIdentifier TRUE)
  elseif(line MATCHES " *<TargetFrameworkVersion>v3.9</TargetFrameworkVersion> *$")
    set(FoundDotNetFrameworkVersion TRUE)
  elseif(line MATCHES " *<TargetFrameworkTargetsVersion>v8.0</TargetFrameworkTargetsVersion> *$")
    set(FoundTargetFrameworkTargetsVersion TRUE)
  elseif( line MATCHES " *<Import Project=\"\\$\\(MSBuildExtensionsPath\\)\\\\Microsoft\\\\\\$\\(TargetFrameworkIdentifier\\)\\\\\\$\\(TargetFrameworkTargetsVersion\\)\\\\Microsoft\\.\\$\\(TargetFrameworkIdentifier\\)\\.CSharp\\.targets\" */> *" )
    set(FoundCFTargetsImport TRUE)
  endif()
endforeach()


if(NOT FoundTargetFrameworkTargetsVersion)
  set(RunCMake_TEST_FAILED "TargetFrameworkIdentifier not found or not set correctly.")
  return()
endif()

if(NOT FoundDotNetFrameworkVersion)
  set(RunCMake_TEST_FAILED "TargetFrameworkVersion not found or not set correctly.")
  return()
endif()

if(NOT FoundTargetFrameworkIdentifier)
  set(RunCMake_TEST_FAILED "TargetFrameworkTargetsVersion not found or not set correctly.")
  return()
endif()

if(NOT FoundCFTargetsImport)
  set(RunCMake_TEST_FAILED "Import of Compact Framework targets file not found or not set correctly.")
  return()
endif()
