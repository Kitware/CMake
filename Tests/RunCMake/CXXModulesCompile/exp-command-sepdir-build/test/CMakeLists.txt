cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_library NONE)

find_package(export_sepdir REQUIRED)
find_package(export_sepdir_cps REQUIRED)

function(test_target target)

  if (NOT TARGET ${target})
    message(FATAL_ERROR
      "Missing imported target")
  endif ()

  get_property(modules TARGET ${target}
    PROPERTY "IMPORTED_CXX_MODULES_NOCONFIG")
  if (modules STREQUAL "")
    message(SEND_ERROR
      "Expected non-empty property value for IMPORTED_CXX_MODULES_NOCONFIG")
  endif ()

endfunction()

test_target(CXXModules::export_sepdir)
test_target(export_sepdir_cps::export_sepdir)
