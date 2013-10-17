
macro(record_compiler_features lang compile_flags feature_list)
  include("${CMAKE_ROOT}/Modules/Compiler/${CMAKE_${lang}_COMPILER_ID}-${lang}-FeatureTests.cmake" OPTIONAL)

  string(TOLOWER ${lang} lang_lc)
  file(REMOVE "${CMAKE_BINARY_DIR}/CMakeFiles/feature_tests${CMAKE_${lang}_OUTPUT_EXTENSION}")
  file(WRITE "${CMAKE_BINARY_DIR}/CMakeFiles/feature_tests.${lang_lc}" "
  extern const char features[] = {\"\"\n")
  foreach(feature ${CMAKE_${lang}_KNOWN_FEATURES})
    if (_cmake_feature_test_${feature})
      if (${_cmake_feature_test_${feature}} STREQUAL 1)
        set(_feature_condition "\"1\" ")
      else()
        set(_feature_condition "#if ${_cmake_feature_test_${feature}}\n\"1\"\n#else\n\"0\"\n#endif\n")
      endif()
      file(APPEND "${CMAKE_BINARY_DIR}/CMakeFiles/feature_tests.${lang_lc}" "\"${lang}_FEATURE:\"\n${_feature_condition}\"${feature}\\n\"\n")
    endif()
  endforeach()
  file(APPEND "${CMAKE_BINARY_DIR}/CMakeFiles/feature_tests.${lang_lc}" "\n};\n")

  if(CMAKE_GENERATOR MATCHES "Unix Makefiles|Ninja")
    # Lightweight version.
    string(REPLACE "<FLAGS>" "${compile_flags}" _compile_object_command "${_CMAKE_${lang}_CREATE_OBJECT_FILE}" )
    string(REPLACE "<SOURCE>" "${CMAKE_BINARY_DIR}/CMakeFiles/feature_tests.${lang_lc}" _compile_object_command "${_compile_object_command}" )
    execute_process(COMMAND "${CMAKE_${lang}_COMPILER}"
      ${_compile_object_command}
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/CMakeFiles"
      ERROR_VARIABLE _error
      OUTPUT_VARIABLE _output
      RESULT_VARIABLE _result
    )
    if(_result EQUAL 0)
      string(REPLACE ";" " " _compile_object_command "${_compile_object_command}")
      set(_output "${CMAKE_${lang}_COMPILER} ${_compile_object_command}\n${_output}")
    endif()
  else()
    file(APPEND "${CMAKE_BINARY_DIR}/CMakeFiles/feature_tests.${lang_lc}" "int main(int, char **) { return 0; }\n")
    try_compile(CMAKE_${lang}_FEATURE_TEST
      ${CMAKE_BINARY_DIR} "${CMAKE_BINARY_DIR}/CMakeFiles/feature_tests.${lang_lc}"
      COMPILE_DEFINITIONS "${compile_flags}"
      OUTPUT_VARIABLE _output
      COPY_FILE "${CMAKE_BINARY_DIR}/CMakeFiles/feature_tests${CMAKE_${lang}_OUTPUT_EXTENSION}"
      COPY_FILE_ERROR _copy_error
      )
    if(CMAKE_${lang}_FEATURE_TEST AND NOT _copy_error)
      set(_result 0)
    else()
      set(_result 255)
    endif()
    unset(CMAKE_${lang}_FEATURE_TEST CACHE)
  endif()
  if (_result EQUAL 0)
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "\n\nDetecting ${lang} [${compile_flags}] compiler features compiled with the following output:\n${_output}\n\n")
    if(EXISTS "${CMAKE_BINARY_DIR}/CMakeFiles/feature_tests${CMAKE_${lang}_OUTPUT_EXTENSION}")
      file(STRINGS "${CMAKE_BINARY_DIR}/CMakeFiles/feature_tests${CMAKE_${lang}_OUTPUT_EXTENSION}"
        features REGEX "${lang}_FEATURE:.*")
      foreach(info ${features})
        file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
          "    Feature record: ${info}\n")
        string(REPLACE "${lang}_FEATURE:" "" info ${info})
        string(SUBSTRING ${info} 0 1 has_feature)
        if(has_feature)
          string(REGEX REPLACE "^1" "" feature ${info})
          list(APPEND ${feature_list} ${feature})
        endif()
      endforeach()
    endif()
  else()
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
      "Detecting ${lang} [${compile_flags}] compiler features failed to compile with the following output:\n${_output}\n${_copy_error}\n\n")
  endif()
endmacro()
