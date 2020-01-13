set(files foo.csproj bar.csproj)

set(inLib1 FALSE)
set(targetFrameworkInLib1 FALSE)

set(inLib2 FALSE)
set(targetFrameworksInLib2 FALSE)

foreach(file ${files})
  set(csProjectFile ${RunCMake_TEST_BINARY_DIR}/${file})

  if(NOT EXISTS "${csProjectFile}")
    set(RunCMake_TEST_FAILED "Project file ${csProjectFile} does not exist.")
    return()
  endif()

  file(STRINGS "${csProjectFile}" lines)

  foreach(line IN LISTS lines)
    if(NOT inLib1)
      if(line MATCHES " *<TargetFrameworkVersion>v4.6.1</TargetFrameworkVersion> *$")
        set(targetFrameworkInLib1  TRUE)
        set(inLib1  TRUE)
      endif()
    elseif(NOT inLib2)
      if(line MATCHES " *<TargetFramework>netcoreapp3.1</TargetFramework> *$")
        set(targetFrameworksInLib2  TRUE)
        set(inLib2 TRUE)
      endif()
    endif()
  endforeach()
endforeach()

if(NOT targetFrameworkInLib1)
  set(RunCMake_TEST_FAILED "TargetFrameworkVersion not set correctly.")
endif()

if(NOT targetFrameworksInLib2)
  set(RunCMake_TEST_FAILED "TargetFramework not set correctly.")
endif()
