cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_library NONE)

find_package(export_interfaces REQUIRED)
find_package(export_interfaces_cps REQUIRED)

function(test_target target set_name has_bmi)
  if (NOT TARGET ${target})
    message(FATAL_ERROR
      "Missing imported target")
  endif ()

  get_property(file_sets TARGET ${target}
    PROPERTY INTERFACE_CXX_MODULE_SETS)
  if (NOT file_sets STREQUAL "${set_name}")
    message(FATAL_ERROR
      "Incorrect exported file sets in ${target}:\n  ${file_sets}")
  endif ()

  get_property(file_set_files TARGET ${target}
    PROPERTY CXX_MODULE_SET_${set_name})
  set(expected_file_set_files
    "${expected_source_dir}/importable.cxx"
    "${expected_source_dir}/subdir/importable.cxx"
    )
  if (NOT file_set_files STREQUAL "${expected_file_set_files}")
    message(FATAL_ERROR
      "Incorrect exported file set paths in ${target}:\n  ${file_set_files}")
  endif ()

  get_property(imported_modules TARGET ${target}
    PROPERTY IMPORTED_CXX_MODULES_DEBUG)
  if(has_bmi)
    set(expected_imported_modules
      "importable=${expected_source_dir}/importable.cxx,${expected_binary_dir}/CMakeFiles/export_interfaces.dir(/Debug)?/importable.(gcm|pcm|ifc)"
      "subdir_importable=${expected_source_dir}/subdir/importable.cxx,${expected_binary_dir}/CMakeFiles/export_interfaces.dir(/Debug)?/subdir_importable.(gcm|pcm|ifc)"
      )
  else()
    set(expected_imported_modules
      "importable=${expected_source_dir}/importable.cxx"
      "subdir_importable=${expected_source_dir}/subdir/importable.cxx"
      )
  endif()
  if (NOT imported_modules MATCHES "^${expected_imported_modules}$")
    message(FATAL_ERROR
      "Incorrect exported modules in ${target}:\n"
      "  ${imported_modules}\n"
      "does not match:\n"
      "  ${expected_imported_modules}"
    )
  endif ()
endfunction()

test_target(CXXModules::export_interfaces "modules" TRUE)
test_target(export_interfaces_cps::export_interfaces "CXX_MODULES" FALSE)
