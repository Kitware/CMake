
include("${CMAKE_ROOT}/Modules/Compiler/${CMAKE_COMPILER_ID}-features.cmake" OPTIONAL)

macro(record_cxx_compiler_features compile_flags feature_list)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/feature_tests.cxx" "
  extern const char features[] = {\n")
  foreach(feature ${CMAKE_CXX_KNOWN_FEATURES})
    set(_define_check "\n\"0\"\n")
    if (feature_test_${feature})
      set(_define_check "#if ${feature_test_${feature}}\n\"1\"\n#else${_define_check}#endif\n")
    endif()
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/feature_tests.cxx" "\"CXX_FEATURE:\"\n${_define_check}\"${feature}\\n\"\n")
  endforeach()
  file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/feature_tests.cxx" "\n};\n")

  execute_process(COMMAND "${CMAKE_CXX_COMPILER}" ${compile_flags} "-c" "${CMAKE_CURRENT_BINARY_DIR}/feature_tests.cxx"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/CMakeFiles"
    RESULT_VARIABLE _result
    OUTPUT_VARIABLE _output
  )
  # We need to capture these when running the process so that the output does
  # not leak and confuse unit tests. Clear the variables so they do not leak
  # to user CMake code either.
  unset(_result)
  unset(_output)
  if (EXISTS "${CMAKE_BINARY_DIR}/CMakeFiles/feature_tests${CMAKE_CXX_OUTPUT_EXTENSION}")
    file(STRINGS "${CMAKE_BINARY_DIR}/CMakeFiles/feature_tests${CMAKE_CXX_OUTPUT_EXTENSION}"
      features REGEX "CXX_FEATURE:.*")
    foreach(info ${features})
      string(REPLACE "CXX_FEATURE:" "" info ${info})
      string(SUBSTRING ${info} 0 1 has_feature)
      if(has_feature)
        string(REGEX REPLACE "^1" "" feature ${info})
        list(APPEND ${feature_list} ${feature})
      endif()
    endforeach()
  endif()
endmacro()
