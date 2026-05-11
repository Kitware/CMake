set(fbuild_bff "${RunCMake_TEST_BINARY_DIR}/fbuild.bff")

if(NOT EXISTS "${fbuild_bff}")
  set(RunCMake_TEST_FAILED "Generator output file is missing:\n ${fbuild_bff}")
  return()
endif()

file(STRINGS "${fbuild_bff}" fbuild_bff_lines
  REGEX "\\.UnityInputExcludedFiles =|unity_.*\\.cpp'|^    \\}")

set(in_excluded_files 0)
set(excluded_sources)
foreach(line IN LISTS fbuild_bff_lines)
  if(line MATCHES "\\.UnityInputExcludedFiles =")
    set(in_excluded_files 1)
  elseif(in_excluded_files AND line MATCHES "^    \\}")
    set(in_excluded_files 0)
  elseif(in_excluded_files AND line MATCHES "unity_([^']+)\\.cpp'")
    list(APPEND excluded_sources "${CMAKE_MATCH_1}")
  endif()
endforeach()

foreach(prop IN ITEMS
    compile_options
    compile_definitions
    include_directories)
  foreach(index IN ITEMS 1 2)
    set(expected_source "${prop}_${index}")
    if(NOT expected_source IN_LIST excluded_sources)
      set(RunCMake_TEST_FAILED
        "Source unity_${prop}_${index}.cpp was not excluded from generated FASTBuild unity files in ${fbuild_bff}")
      return()
    endif()
  endforeach()
endforeach()
