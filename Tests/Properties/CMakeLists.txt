# a simple CXX only test case
project (Properties)

# these first three tests really test both properties and the management of 
# cmSourceFile objects by CMake. 

# test properties on a build tree file that is relative (yuck)
configure_file(properties.h.in "${Properties_BINARY_DIR}/properties.h")
set_source_files_properties(properties.h PROPERTIES TEST1 1)
get_source_file_property(RESULT1 properties.h TEST1)

# test properties on a headerfile in the source tree 
# accessed without an extenion (also yuck)
set_source_files_properties(properties2 PROPERTIES TEST2 1)
get_source_file_property(RESULT2 properties2 TEST2)

# test properties on a relative source that is not generated
set_source_files_properties(SubDir/properties3.cxx PROPERTIES TEST3 1)
get_source_file_property(RESULT3 SubDir/properties3.cxx TEST3)

include_directories("${Properties_SOURCE_DIR}" "${Properties_BINARY_DIR}")

if (RESULT1 AND RESULT2 AND RESULT3)
  add_executable (Properties SubDir/properties3.cxx)
else (RESULT1 AND RESULT2 AND RESULT3)
  message("Error: test results are TEST1=${TEST1} TEST2=${TEST2} TEST3=${TEST3}")
endif (RESULT1 AND RESULT2 AND RESULT3)
