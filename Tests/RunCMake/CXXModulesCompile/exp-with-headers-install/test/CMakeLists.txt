cmake_minimum_required(VERSION 3.28)
project(cxx_modules_library NONE)

find_package(export_with_headers REQUIRED)
find_package(export_with_headers_cps REQUIRED)

set(include_dir "${export_with_headers_DIR}")
cmake_path(GET include_dir PARENT_PATH include_dir)
cmake_path(GET include_dir PARENT_PATH include_dir)
cmake_path(GET include_dir PARENT_PATH include_dir)
string(APPEND include_dir "/include")

function(test_target target incdir)

  if (NOT TARGET ${target})
    message(FATAL_ERROR
      "Missing imported target")
  endif ()

  get_property(iface_includes TARGET ${target}
    PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
  if (NOT iface_includes STREQUAL incdir)
    message(FATAL_ERROR
      "Incorrect include interface for ${target}:\n  ${iface_includes}")
  endif ()

endfunction(test_target)

test_target(CXXModules::export_with_headers "$<BUILD_INTERFACE:${include_dir}>")
test_target(export_with_headers_cps::export_with_headers "${include_dir}")
