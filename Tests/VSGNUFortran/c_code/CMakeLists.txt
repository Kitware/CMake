add_executable(c_using_fortran main.c)
# force the executable to be put out of Debug/Release dir
# because gmake build of fortran will not be in a config
# directory.
if(CMAKE_CONFIGURATION_TYPES)
  set_target_properties(c_using_fortran PROPERTIES
    OUTPUT_NAME ../c_using_fortran)
endif()

target_link_libraries(c_using_fortran hello)
