enable_language(C)
enable_testing()

add_executable(main main.c)
set_property(TARGET main PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
if(NOT CMAKE_EXECUTABLE_SUFFIX)
  set_property(TARGET main PROPERTY SUFFIX .out)
endif()

# Test discovery of binary under working directory with just its name.
add_test(NAME main COMMAND $<TARGET_FILE_NAME:main> WORKING_DIRECTORY bin)
