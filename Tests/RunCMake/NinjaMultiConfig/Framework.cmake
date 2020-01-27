enable_language(C)

set(header "${CMAKE_CURRENT_BINARY_DIR}/header.h")
file(GENERATE
  OUTPUT "${header}"
  CONTENT "/* foo */"
  CONDITION "$<CONFIG:Release>"
  )
add_library(framework SHARED "${header}" empty.c)

set_property(TARGET framework PROPERTY FRAMEWORK ON)
set_property(TARGET framework APPEND PROPERTY PUBLIC_HEADER ${header})

set_target_properties(framework PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "lib"
  LIBRARY_OUTPUT_DIRECTORY_DEBUG "lib"
  LIBRARY_OUTPUT_DIRECTORY_RELEASE "lib"
  DEBUG_POSTFIX "_debug"
  )

include(${CMAKE_CURRENT_LIST_DIR}/Common.cmake)
generate_output_files(framework)
