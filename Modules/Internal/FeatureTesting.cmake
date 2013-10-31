
macro(record_compiler_features lang compile_flags feature_list)
  include("${CMAKE_ROOT}/Modules/Compiler/${CMAKE_${lang}_COMPILER_ID}-${lang}-FeatureTests.cmake" OPTIONAL)

  string(TOLOWER ${lang} lang_lc)
  file(REMOVE "${CMAKE_BINARY_DIR}/CMakeFiles/feature_tests${CMAKE_${lang}_OUTPUT_EXTENSION}")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/feature_tests.${lang_lc}" "
  extern const char features[] = {\n")
  foreach(feature ${CMAKE_${lang}_KNOWN_FEATURES})
    set(_define_check "\n\"0\"\n")
    if (_cmake_feature_test_${feature})
      set(_define_check "#if ${_cmake_feature_test_${feature}}\n\"1\"\n#else${_define_check}#endif\n")
    endif()
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/feature_tests.${lang_lc}" "\"${lang}_FEATURE:\"\n${_define_check}\"${feature}\\n\"\n")
  endforeach()
  file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/feature_tests.${lang_lc}" "\n};\n")

  string(REPLACE "<FLAGS>" "${compile_flags}" _compile_object_command "${_CMAKE_${lang}_CREATE_OBJECT_FILE}" )
  string(REPLACE "<SOURCE>" "${CMAKE_CURRENT_BINARY_DIR}/feature_tests.${lang_lc}" _compile_object_command "${_compile_object_command}" )
  execute_process(COMMAND ${_compile_object_command}
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/CMakeFiles"
    ERROR_VARIABLE _error
    OUTPUT_VARIABLE _output
    RESULT_VARIABLE _result
  )
  # We need to capture these when running the process so that the output does
  # not leak and confuse unit tests. Clear the variables so they do not leak
  # to user CMake code either.
  unset(_error)
  unset(_output)
  if (_result EQUAL 0 AND EXISTS "${CMAKE_BINARY_DIR}/CMakeFiles/feature_tests${CMAKE_${lang}_OUTPUT_EXTENSION}")
    file(STRINGS "${CMAKE_BINARY_DIR}/CMakeFiles/feature_tests${CMAKE_${lang}_OUTPUT_EXTENSION}"
      features REGEX "${lang}_FEATURE:.*")
    foreach(info ${features})
      string(REPLACE "${lang}_FEATURE:" "" info ${info})
      string(SUBSTRING ${info} 0 1 has_feature)
      if(has_feature)
        string(REGEX REPLACE "^1" "" feature ${info})
        list(APPEND ${feature_list} ${feature})
      endif()
    endforeach()
  endif()
endmacro()
