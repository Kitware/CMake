cmake_minimum_required(VERSION 3.28)
project(cxx_modules_library NONE)

find_package(export_transitive_modules REQUIRED)
find_package(export_transitive_modules_cps REQUIRED)

function(test_namespace namespace namespace1)
  if (NOT TARGET ${namespace1}::export_transitive_modules1)
    message(FATAL_ERROR
      "Missing transitive imported target")
  endif ()

  if (NOT TARGET ${namespace}::export_transitive_modules)
    message(FATAL_ERROR
      "Missing imported target")
  endif ()
endfunction()

test_namespace(CXXModules CXXModules)
test_namespace(export_transitive_modules_cps export_transitive_modules1_cps)
