cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_library NONE)

find_package(export_include_directories REQUIRED)
find_package(export_include_directories_cps REQUIRED)

function(test_target target set_name)
  if (NOT TARGET ${target})
    message(FATAL_ERROR
      "Missing imported target")
  endif ()

  get_property(file_sets TARGET ${target}
    PROPERTY INTERFACE_CXX_MODULE_SETS)
  if (NOT file_sets STREQUAL ${set_name})
    message(FATAL_ERROR
      "Incorrect exported file sets in ${target}:\n  ${file_sets}")
  endif ()
endfunction()

test_target(CXXModules::export_include_directories "modules")
test_target(export_include_directories_cps::export_include_directories "CXX_MODULES")
