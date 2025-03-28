include(CMakeFindFrameworks OPTIONAL RESULT_VARIABLE found)
if(NOT should_find AND found)
  message(FATAL_ERROR
    "The CMakeFindFrameworks module should not have been found, but it was."
  )
endif()
if(should_find AND NOT found)
  message(FATAL_ERROR
    "The CMakeFindFrameworks module should have been found, but it was not."
  )
endif()
include(${CMAKE_ROOT}/Modules/CMakeFindFrameworks.cmake)
