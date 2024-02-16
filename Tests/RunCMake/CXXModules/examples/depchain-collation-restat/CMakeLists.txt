cmake_minimum_required(VERSION 3.28)
project(cxx_modules_depchain_collation_restat CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(depchain_with_collation_restat)
target_sources(depchain_with_collation_restat
  PUBLIC
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        importable.cxx)
target_compile_features(depchain_with_collation_restat PUBLIC cxx_std_20)

add_executable(depchain_collation_restat)
target_sources(depchain_collation_restat
  PRIVATE
    main.cxx)
target_link_libraries(depchain_collation_restat
  PRIVATE
    depchain_with_collation_restat)
add_test(NAME depchain_collation_restat COMMAND depchain_collation_restat)
