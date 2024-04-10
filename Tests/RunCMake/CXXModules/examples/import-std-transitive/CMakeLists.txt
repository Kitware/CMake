set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

cmake_minimum_required(VERSION 3.29)

if (EXPORT_NO_STD)
  # Block making C++ `import std` targets.
  add_library(__CMAKE::CXX23 IMPORTED INTERFACE)
endif ()

project(cxx_modules_import_std_transitive CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

if (EXPORT_NO_STD)
  set(package "import_std_export_no_std")
else ()
  set(package "import_std_not_in_export")
endif ()
find_package("${package}" REQUIRED)

set(CMAKE_CXX_MODULE_STD 0)

add_executable(main
  main.cxx)
target_link_libraries(main PRIVATE "CXXModules::${package}")

add_test(NAME main COMMAND main)
