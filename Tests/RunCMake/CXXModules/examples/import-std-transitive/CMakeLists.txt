set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  "a9e1cf81-9932-4810-974b-6eccaf14e457")

cmake_minimum_required(VERSION 3.29)

if (EXPORT_NO_STD)
  # Block making C++ `import std` targets.
  add_library(__CMAKE::CXX23 IMPORTED INTERFACE)
  add_library(__CMAKE::CXX26 IMPORTED INTERFACE)
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
