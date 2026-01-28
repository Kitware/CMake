cmake_minimum_required(VERSION 3.24)
project(cxx_modules_library NONE)

find_package(export_include_directories REQUIRED)
find_package(export_include_directories_cps REQUIRED)

function(test_target target)
  if (NOT TARGET ${target})
    message(FATAL_ERROR
      "Missing imported target")
  endif ()

  get_property(include_directories TARGET ${target}
    PROPERTY IMPORTED_CXX_MODULES_INCLUDE_DIRECTORIES)
  foreach (include_directory IN LISTS include_directories)
    if (NOT EXISTS "${include_directory}")
      message(FATAL_ERROR
        "Missing include directory in C++ module interface ${target}:\n  ${include_directory}")
    endif ()
  endforeach ()
endfunction()

test_target(CXXModules::export_include_directories)
test_target(export_include_directories_cps::export_include_directories)
