set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  "25d6f6aa-be65-4692-b44e-87b23e96d4e1")

cmake_minimum_required(VERSION 3.29)

project(cxx_modules_import_std_transitive CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

if (EXPORT_NO_STD)
  set(package "import_std_export_no_std")
else ()
  set(package "import_std_not_in_export")
endif ()
find_package("${package}" REQUIRED)
find_package("${package}_cps" REQUIRED)

add_executable(main
  main.cxx)
target_compile_features(main PRIVATE cxx_std_23)
target_link_libraries(main PRIVATE
  "CXXModules::${package}"
  "${package}_cps::${package}")

add_test(NAME main COMMAND main)
