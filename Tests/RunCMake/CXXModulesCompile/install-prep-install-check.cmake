if (EXISTS "${prefix}/artifacts")
  list(APPEND RunCMake_TEST_FAILED "Installation included an excluded archive.")
endif ()

if (kind STREQUAL "GeneratedSource")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E compare_files --ignore-eol
      "${RunCMake_TEST_SOURCE_DIR}/importable.cxx"
      "${prefix}/sources/generated.cxx"
    RESULT_VARIABLE result)
  if (NOT result EQUAL 0)
    list(APPEND RunCMake_TEST_FAILED "The installed generated module source does not match the input.")
  endif ()
  string(REPLACE ";" "\n  " RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}")
  return ()
endif ()

if (kind STREQUAL "BMI")
  set(bmi)
  foreach (ext IN ITEMS gcm ifc pcm)
    set(path "${prefix}/lib/cxx/bmi/importable.${ext}")
    if (EXISTS "${path}" AND NOT IS_DIRECTORY "${path}")
      set(bmi "${path}")
      break ()
    endif ()
  endforeach ()
  if (NOT bmi)
    list(APPEND RunCMake_TEST_FAILED "The prepared BMI was not installed.")
  endif ()
else ()
  set(metadata "${prefix}/lib/cmake/export_interfaces/cxx-modules/target-export__interfaces-${config}.cmake")
  if (NOT EXISTS "${metadata}")
    list(APPEND RunCMake_TEST_FAILED "The prepared module export metadata was not installed.")
    return ()
  endif ()
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E compare_files --ignore-eol
      "${CMAKE_CURRENT_LIST_DIR}/expect/install-prep-export-interfaces-${config}.cmake"
      "${metadata}"
    RESULT_VARIABLE result)
  if (NOT result EQUAL 0)
    list(APPEND RunCMake_TEST_FAILED "Installed module export metadata does not match the expected file.")
  endif ()
endif ()

if (EXISTS "${prefix}/${source_destination}")
  list(APPEND RunCMake_TEST_FAILED "Installation included an excluded source file set.")
endif ()

string(REPLACE ";" "\n  " RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}")
