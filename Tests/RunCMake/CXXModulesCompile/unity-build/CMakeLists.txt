cmake_minimum_required(VERSION 3.28)
project(cxx_modules_unity CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

set(CMAKE_UNITY_BUILD 1)

add_executable(unity)
target_sources(unity
  PRIVATE
    main.cxx
    unity1.cxx
    unity2.cxx
  PRIVATE
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        importable.cxx)
target_compile_features(unity PUBLIC cxx_std_20)

set_property(SOURCE unity1.cxx unity2.cxx
  PROPERTY
    CXX_SCAN_FOR_MODULES 0)

add_test(NAME unity COMMAND unity)
