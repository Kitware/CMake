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
    PROPERTY IMPORTED_CXX_MODULES_DEBUG)
  set(expected_modules "importable=${expected_dir}/importable.cxx")
  if (NOT modules STREQUAL "${expected_modules}")
    message(FATAL_ERROR
      "Incorrect exported modules in ${target}:\n"
      "  ${modules}\n"
      "does not match:\n"
      "  ${expected_modules}")
  endif ()

endfunction()

test_target(CXXModules::export_sepdir)
test_target(export_sepdir_cps::export_sepdir)
