set(CMAKE_UNITY_BUILD ON)

add_executable(main main.cpp)

target_sources(main PRIVATE FILE_SET s1 TYPE SOURCES FILES unity_compile_options_1.cpp
                                                           unity_compile_options_2.cpp)
set_property(FILE_SET s1 TARGET main PROPERTY COMPILE_OPTIONS -DUNITY_COMPILE_OPTIONS)

target_sources(main PRIVATE FILE_SET s2 TYPE SOURCES FILES unity_compile_definitions_1.cpp
                                                           unity_compile_definitions_2.cpp)
set_property(FILE_SET s2 TARGET main PROPERTY COMPILE_DEFINITIONS UNITY_COMPILE_DEFINITIONS)


target_sources(main PRIVATE FILE_SET s3 TYPE SOURCES FILES unity_include_directories_1.cpp
                                                           unity_include_directories_2.cpp)
set_property(FILE_SET s3 TARGET main PROPERTY INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}")
