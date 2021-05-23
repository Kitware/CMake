foreach(test_language C CXX)
  enable_language(${test_language})
  if(DEFINED CMAKE_${test_language}_STANDARD_DEFAULT
      AND NOT CMAKE_${test_language}_COMPILE_FEATURES)
    message(FATAL_ERROR "Compile features not found for ${test_language}")
  endif()
endforeach()
