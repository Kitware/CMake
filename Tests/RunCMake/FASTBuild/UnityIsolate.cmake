set(CMAKE_UNITY_BUILD ON)
add_executable(main
    main.cpp
    some_source_file_1.cpp
    some_source_file_2.cpp
    some_source_file_3.cpp
    some_source_file_4.cpp
)

set_source_files_properties(
    some_source_file_1.cpp
    some_source_file_4.cpp

    TARGET_DIRECTORY
        main
    PROPERTIES
        SKIP_UNITY_BUILD_INCLUSION ON
)
