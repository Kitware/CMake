macro(checkCompileAsWinRT projectPath)
  if(RunCMake_TEST_FAILED)
    return()
  endif()

  if (NOT EXISTS "${projectPath}")
    set(RunCMake_TEST_FAILED "Project file ${projectPath} does not exist.")
    return()
  endif()

  get_filename_component(projectName "${projectPath}" NAME_WE)

  cmake_parse_arguments("" "" "GLOBAL" "OVERRIDES_ENABLE;OVERRIDES_DISABLE" ${ARGN})

  unset(sourceOverride)

  file(STRINGS "${projectPath}" lines)
  set(foundGlobalWinRT false)

  foreach(line IN LISTS lines)
    if(line MATCHES "^ *<CompileAsWinRT( Condition=\"[^\\\"]+\")?>(true|false)</CompileAsWinRT>$")
      set(value ${CMAKE_MATCH_2})

      if(sourceOverride)
        set(expectedList)

        if(value)
          set(expectedList _OVERRIDES_ENABLE)
        else()
          set(expectedList _OVERRIDES_DISABLE)
        endif()

        if(NOT sourceOverride IN_LIST ${expectedList})
          set(RunCMake_TEST_FAILED
            "${projectName}: Unexpected CompileAsWinRT override ${value} for ${sourceOverride}")
          return()
        endif()
      else()
        if (NOT _GLOBAL STREQUAL value)
          set(RunCMake_TEST_FAILED
            "${projectName}: Global CompileAsWinRT value is ${value}, but expected ${_GLOBAL}")
          return()
        endif()

        set(foundGlobalWinRT true)
      endif()
    elseif(line MATCHES "^ *<ClCompile Include=\"([^\"]+)\">$")
      get_filename_component(sourceOverride "${CMAKE_MATCH_1}" NAME)
    elseif(line MATCHES "^ *</ClCompile>$")
      unset(sourceOverride)
    endif()
  endforeach()

  if(NOT foundGlobalWinRT AND DEFINED _GLOBAL)
    set(RunCMake_TEST_FAILED "${projectName}: Global CompileAsWinRT not found or have invalid value, but expected")
    return()
  endif()
endmacro()

checkCompileAsWinRT("${RunCMake_TEST_BINARY_DIR}/noFlagOnlyC.vcxproj" GLOBAL false)
checkCompileAsWinRT("${RunCMake_TEST_BINARY_DIR}/noFlagMixedCAndCxx.vcxproj" GLOBAL false)
checkCompileAsWinRT("${RunCMake_TEST_BINARY_DIR}/noFlagOnlyCxx.vcxproj" GLOBAL false)

checkCompileAsWinRT("${RunCMake_TEST_BINARY_DIR}/flagOnlyC.vcxproj" GLOBAL true OVERRIDES_DISABLE empty.c)
checkCompileAsWinRT("${RunCMake_TEST_BINARY_DIR}/flagMixedCAndCxx.vcxproj" GLOBAL true OVERRIDES_DISABLE empty.c)
checkCompileAsWinRT("${RunCMake_TEST_BINARY_DIR}/flagOnlyCxx.vcxproj" GLOBAL true)
