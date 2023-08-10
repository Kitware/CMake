
macro(returnOnError errorMsg)
  if(NOT "${errorMsg}" STREQUAL "")
    set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}\n${errorMsg}" PARENT_SCOPE)
    return()
  endif()
endmacro()

function(getTargetFlags mainTarget projFlagsVar flagsVar errorVar)
  # The flags variables in the project file might span over multiple lines
  # so we can't easily read the flags directly from there. Instead, we use
  # the xcodebuild -showBuildSettings option to report it on a single line.
  execute_process(
    COMMAND ${CMAKE_COMMAND}
            --build ${RunCMake_TEST_BINARY_DIR}
            --target ${mainTarget}
            --config Debug
            --
            -showBuildSettings
    COMMAND grep ${projFlagsVar}
    OUTPUT_VARIABLE flagsContents
    RESULT_VARIABLE result
  )

  if(result)
    set(${errorVar} "Failed to get flags for ${mainTarget}: ${result}" PARENT_SCOPE)
  else()
    unset(${errorVar} PARENT_SCOPE)
  endif()
  set(${flagsVar} "${flagsContents}" PARENT_SCOPE)
endfunction()

function(checkFlags projFlagsVar mainTarget present absent)
  getTargetFlags(${mainTarget} ${projFlagsVar} flags errorMsg)
  returnOnError("${errorMsg}")

  foreach(linkTo IN LISTS present)
    string(REGEX MATCH "${linkTo}" result "${flags}")
    if("${result}" STREQUAL "")
      string(APPEND RunCMake_TEST_FAILED
        "\n${mainTarget} ${projFlagsVar} is missing ${linkTo}"
      )
    endif()
  endforeach()

  foreach(linkTo IN LISTS absent)
    string(REGEX MATCH "${linkTo}" result "${flags}")
    if(NOT "${result}" STREQUAL "")
      string(APPEND RunCMake_TEST_FAILED
        "\n${mainTarget} ${projFlagsVar} unexpectedly contains ${linkTo}"
      )
    endif()
  endforeach()

  set(RunCMake_TEST_FAILED ${RunCMake_TEST_FAILED} PARENT_SCOPE)
endfunction()
