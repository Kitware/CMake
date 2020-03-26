#
# Check C# VS project for required elements.
#
set(csProjectFile "${RunCMake_TEST_BINARY_DIR}/foo.csproj")
if(NOT EXISTS "${csProjectFile}")
  set(RunCMake_TEST_FAILED "Project file ${csProjectFile} does not exist.")
  return()
endif()


set(inDebug FALSE)
set(inRelease FALSE)
set(debugOK FALSE)
set(releaseOK FALSE)


file(STRINGS "${csProjectFile}" lines)
foreach(line IN LISTS lines)
  #message(STATUS ${line})
  if(line MATCHES "^ *<PropertyGroup .*Debug\\|(Win32|x64).*")
    set(inDebug TRUE)
  elseif(line MATCHES "^ *<PropertyGroup .*Release\\|(Win32|x64).*")
    set(inRelease TRUE)
  elseif(line MATCHES "^ *</PropertyGroup> *$")
    set(inRelease FALSE)
    set(inDebug  FALSE)
  elseif(inDebug AND
     (line MATCHES "^ *<DefineConstants>.*MY_FOO_DEFINE.*</DefineConstants> *$") AND
     (line MATCHES "^ *<DefineConstants>.*DEFINE_ONLY_FOR_DEBUG.*</DefineConstants> *$") AND
     (NOT (line MATCHES "^ *<DefineConstants>.*DEFINE_ONLY_FOR_RELEASE.*</DefineConstants> *$")) AND
     (NOT (line MATCHES "^ *<DefineConstants>.*MY_BAR_ASSIGNMENT=bar.*</DefineConstants> *$"))
    )
    set(debugOK TRUE)
  elseif(inRelease AND
     (line MATCHES "^ *<DefineConstants>.*MY_FOO_DEFINE.*</DefineConstants> *$") AND
     (line MATCHES "^ *<DefineConstants>.*DEFINE_ONLY_FOR_RELEASE.*</DefineConstants> *$") AND
     (NOT (line MATCHES "^ *<DefineConstants>.*DEFINE_ONLY_FOR_DEBUG.*</DefineConstants> *$")) AND
     (NOT (line MATCHES "^ *<DefineConstants>.*MY_BAR_ASSIGNMENT=bar.*</DefineConstants> *$"))
    )
    set(releaseOK TRUE)
  endif()
endforeach()

function(print_csprojfile)
  file(STRINGS "${csProjectFile}" lines)
  foreach(line IN LISTS lines)
    message(STATUS ${line})
  endforeach()
endfunction()


if(NOT debugOK)
  message(STATUS "Failed to set Debug configuration defines correctly.")
  set(RunCMake_TEST_FAILED "Failed to set Debug configuration defines correctly.")
  print_csprojfile()
  return()
endif()

if(NOT releaseOK)
  message(STATUS "Failed to set Release configuration defines correctly.")
  set(RunCMake_TEST_FAILED "Failed to set Release configuration defines correctly.")
  print_csprojfile()
  return()
endif()
