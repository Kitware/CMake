add_library(autoexport2 SHARED sub.cxx)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # Try msvc "big" object format.
  target_compile_options(autoexport2 PRIVATE /bigobj)
endif()
add_custom_command(TARGET autoexport2 PRE_LINK COMMAND ${CMAKE_COMMAND} -E true)
