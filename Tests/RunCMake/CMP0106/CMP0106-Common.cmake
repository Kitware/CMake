include(Documentation OPTIONAL RESULT_VARIABLE found)
if (NOT should_find AND found)
  message(FATAL_ERROR
    "The Documentation module should not have been found, but it was.")
endif ()
if (should_find AND NOT found)
  message(FATAL_ERROR
    "The Documentation module should have been found, but it was not.")
endif ()
include(${CMAKE_ROOT}/Modules/Documentation.cmake)
