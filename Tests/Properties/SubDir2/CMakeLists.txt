
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/../subdirtest.cxx"
  PROPERTIES COMPILE_DEFINITIONS SUBDIR_TEST)

add_executable(subdirtest "${CMAKE_CURRENT_SOURCE_DIR}/../subdirtest.cxx")

# For set_property
add_library(set_prop_lib_1 SHARED)
foreach(i RANGE 1 6)
    generate_file_for_set_property_test(${i} set_prop_lib_1)
endforeach()

# For set_source_files_properties
foreach(i RANGE 10 12)
    generate_file_for_set_property_test(${i} set_prop_lib_1)
endforeach()

# For set_source_files_properties + multiple files + absolute directory path
add_library(set_prop_lib_2 SHARED)
foreach(i RANGE 20 21)
    generate_file_for_set_property_test(${i} set_prop_lib_1)
endforeach()

# For set_source_files_properties + multiple files + multiple target directories
add_library(set_prop_lib_3 SHARED)
generate_file_for_set_property_test(22 set_prop_lib_2)
generate_file_for_set_property_test(23 set_prop_lib_3)

# For set_source_files_properties + multiple files in multiple directories
generate_file_for_set_property_test(31 set_prop_lib_3)

# For specifying both DIRECTORY and TARGET_DIRECTORY
target_sources(set_prop_lib_3 PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../src32.cpp")
target_sources(set_prop_lib_3 PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../src33.cpp")
