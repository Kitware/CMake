set(CMAKE_UNITY_BUILD ON)
add_executable(main
    main.cpp)

target_sources(main PRIVATE FILE_SET s1 TYPE SOURCES FILES some_source_file_1.cpp
                                                           some_source_file_4.cpp)
set_property(FILE_SET s1 TARGET main PROPERTY SKIP_UNITY_BUILD_INCLUSION ON)

target_sources(main PRIVATE FILE_SET s2 TYPE SOURCES FILES some_source_file_2.cpp
                                                           some_source_file_3.cpp)
