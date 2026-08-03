set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  "25d6f6aa-be65-4692-b44e-87b23e96d4e1")

cmake_minimum_required(VERSION 3.29)
project(cxx_modules_library CXX)

find_package(import_std_shared REQUIRED)

if (NOT TARGET CXXModules::import_std_shared)
  message(FATAL_ERROR "Missing imported target")
endif ()

add_executable(main main.cxx)
target_compile_features(main PRIVATE cxx_std_23)
target_link_libraries(main PRIVATE CXXModules::import_std_shared)
