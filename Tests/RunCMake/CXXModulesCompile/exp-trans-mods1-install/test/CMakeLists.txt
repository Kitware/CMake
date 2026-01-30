cmake_minimum_required(VERSION 3.28)
project(cxx_modules_library NONE)

find_package(export_transitive_modules1 REQUIRED)
find_package(export_transitive_modules1_cps REQUIRED)

function(test_namespace namespace)
  if (NOT TARGET ${namespace}::export_transitive_modules1)
    message(FATAL_ERROR
      "Missing imported target")
  endif ()
endfunction()

test_namespace(CXXModules)
test_namespace(export_transitive_modules1_cps)
