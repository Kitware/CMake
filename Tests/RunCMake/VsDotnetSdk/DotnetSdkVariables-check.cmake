set(files foo.csproj bar.csproj baz.csproj)

set(inLib1 FALSE)
set(dotnetSdkInLib1 FALSE)

set(inLib2 FALSE)
set(dotnetSdkWebInLib2 FALSE)

set(inLib3 FALSE)
set(classicProjInLib3 FALSE)

foreach(file ${files})
  set(csProjectFile ${RunCMake_TEST_BINARY_DIR}/${file})

  if(NOT EXISTS "${csProjectFile}")
    set(RunCMake_TEST_FAILED "Project file ${csProjectFile} does not exist.")
    return()
  endif()

  file(STRINGS "${csProjectFile}" lines)

  foreach(line IN LISTS lines)
    if(NOT inLib1)
      if(line MATCHES "<Project Sdk=\"Microsoft\.NET\.Sdk\">")
        set(dotnetSdkInLib1 TRUE)
        set(inLib1  TRUE)
      endif()
    elseif(NOT inLib2)
      if(line MATCHES "<Project Sdk=\"Microsoft\.NET\.Sdk\.Web\">")
        set(dotnetSdkWebInLib2 TRUE)
        set(inLib2 TRUE)
      endif()
    elseif(NOT inLib3)
      if(line MATCHES "<Project DefaultTargets=\"Build\" ToolsVersion=\"")
        set(classicProjInLib3 TRUE)
        set(inLib3 TRUE)
      endif()
    endif()
  endforeach()
endforeach()

if(NOT dotnetSdkInLib1)
  set(RunCMake_TEST_FAILED ".Net SDK not set correctly.")
endif()

if(NOT dotnetSdkWebInLib2)
  set(RunCMake_TEST_FAILED ".Net Web SDK not set correctly.")
endif()

if(NOT classicProjInLib3)
  set(RunCMake_TEST_FAILED "Empty DOTNET_SDK doesn't build Classic project.")
endif()
