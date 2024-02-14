cmake_minimum_required(VERSION 3.28)
project(cxx_modules_depchain_modmap CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_executable(depchain_modmap)
target_sources(depchain_modmap
  PRIVATE
    main.cxx
  PRIVATE
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        importable.cxx)
target_compile_features(depchain_modmap PUBLIC cxx_std_20)

add_test(NAME depchain_modmap COMMAND depchain_modmap)
