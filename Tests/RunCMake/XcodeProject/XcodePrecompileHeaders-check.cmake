set(pch_header "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/tgt.dir/Debug/cmake_pch.hxx")

if(NOT EXISTS "${pch_header}")
  set(RunCMake_TEST_FAILED "Generated PCH header ${pch_header} does not exist.")
  return()
endif()

set(tgt_project "${RunCMake_TEST_BINARY_DIR}/XcodePrecompileHeaders.xcodeproj/project.pbxproj")
if (NOT EXISTS "${tgt_project}")
  set(RunCMake_TEST_FAILED "Generated project file ${tgt_project} doesn't exist.")
  return()
endif()

file(STRINGS ${tgt_project} tgt_projects_strings)

foreach(line IN LISTS tgt_projects_strings)
  if (line MATCHES "GCC_PRECOMPILE_PREFIX_HEADER = YES;")
    set(have_pch_prefix ON)
  endif()

  string(FIND "${line}" "GCC_PREFIX_HEADER = \"${pch_header}\";" find_pos)
  if (NOT find_pos EQUAL "-1")
    set(have_pch_header ON)
  endif()
endforeach()

if (NOT have_pch_prefix)
  set(RunCMake_TEST_FAILED "Generated project should have the GCC_PRECOMPILE_PREFIX_HEADER = YES; line.")
  return()
endif()

if (NOT have_pch_header)
  set(RunCMake_TEST_FAILED "Generated project should have the GCC_PREFIX_HEADER = \"${pch_header}\"; line.")
  return()
endif()
