set(pch_header "CMakeFiles/tgt.dir/Debug/cmake_pch.hxx")
set(pch_source [=[CMakeFiles\\tgt.dir\\cmake_pch.cxx]=])

if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/${pch_header}")
  set(RunCMake_TEST_FAILED "Generated PCH header ${pch_header} does not exist.")
  return()
endif()
if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/${pch_source}")
  set(RunCMake_TEST_FAILED "Generated PCH header ${pch_source} does not exist.")
  return()
endif()

set(tgt_project "${RunCMake_TEST_BINARY_DIR}/tgt.vcxproj")
if (NOT EXISTS "${tgt_project}")
  set(RunCMake_TEST_FAILED "Generated project file ${tgt_project} doesn't exist.")
  return()
endif()

file(STRINGS ${tgt_project} tgt_projects_strings)

foreach(line IN LISTS tgt_projects_strings)
  if (line MATCHES "<PrecompiledHeader.*>Use</PrecompiledHeader>")
    set(have_pch_use ON)
  endif()

  if (line MATCHES "<PrecompiledHeader.*>Create</PrecompiledHeader>")
    set(have_pch_create ON)
  endif()

  if (line MATCHES "<PrecompiledHeaderFile.*>.*${pch_header}</PrecompiledHeaderFile>")
    set(have_pch_header ON)
  endif()

  if (line MATCHES "<ForcedIncludeFiles.*>.*${pch_header}.*</ForcedIncludeFiles>")
    set(have_force_pch_header ON)
  endif()

  if (line MATCHES "<ClCompile Include=.*${pch_source}\">")
    set(have_pch_source_compile ON)
  endif()
endforeach()

if (NOT have_pch_use)
  set(RunCMake_TEST_FAILED "Generated project should have the <PrecompiledHeader>Use</PrecompiledHeader> block.")
  return()
endif()

if (NOT have_pch_create)
  set(RunCMake_TEST_FAILED "Generated project should have the <PrecompiledHeader>Create</PrecompiledHeader> block.")
  return()
endif()

if (NOT have_pch_header)
  set(RunCMake_TEST_FAILED "Generated project should have the <PrecompiledHeaderFile>${pch_header}</PrecompiledHeaderFile> block.")
  return()
endif()

if (NOT have_force_pch_header)
  set(RunCMake_TEST_FAILED "Generated project should have the <ForcedIncludeFiles>${pch_header}</ForcedIncludeFiles> block.")
  return()
endif()

if (NOT have_pch_source_compile)
  set(RunCMake_TEST_FAILED "Generated project should have the <ClCompile Include=\"${pch_source}\"> block.")
  return()
endif()
