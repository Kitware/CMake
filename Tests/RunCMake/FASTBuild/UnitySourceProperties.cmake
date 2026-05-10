set(CMAKE_UNITY_BUILD ON)

add_executable(main
  main.cpp
  unity_compile_options_1.cpp
  unity_compile_options_2.cpp
  unity_compile_definitions_1.cpp
  unity_compile_definitions_2.cpp
  unity_compile_flags_1.cpp
  unity_compile_flags_2.cpp
  unity_include_directories_1.cpp
  unity_include_directories_2.cpp
)

set_source_files_properties(
  unity_compile_options_1.cpp
  unity_compile_options_2.cpp
  TARGET_DIRECTORY
    main
  PROPERTIES
    COMPILE_OPTIONS -DUNITY_COMPILE_OPTIONS
)

set_source_files_properties(
  unity_compile_definitions_1.cpp
  unity_compile_definitions_2.cpp
  TARGET_DIRECTORY
    main
  PROPERTIES
    COMPILE_DEFINITIONS UNITY_COMPILE_DEFINITIONS
)

set_source_files_properties(
  unity_compile_flags_1.cpp
  unity_compile_flags_2.cpp
  TARGET_DIRECTORY
    main
  PROPERTIES
    COMPILE_FLAGS -DUNITY_COMPILE_FLAGS
)

set_source_files_properties(
  unity_include_directories_1.cpp
  unity_include_directories_2.cpp
  TARGET_DIRECTORY
    main
  PROPERTIES
    INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}"
)
