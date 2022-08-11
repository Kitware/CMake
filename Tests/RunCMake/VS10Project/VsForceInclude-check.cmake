set(tgt_project "${RunCMake_TEST_BINARY_DIR}/tgt.vcxproj")
if (NOT EXISTS "${tgt_project}")
  set(RunCMake_TEST_FAILED "Generated project file does not exist:\n ${tgt_project}\n")
  return()
endif()

file(STRINGS ${tgt_project} tgt_projects_strings REGEX ForcedIncludeFiles)

foreach(line IN LISTS tgt_projects_strings)
  if (line MATCHES "<ForcedIncludeFiles>force_include_1.h;force_include_2.h</ForcedIncludeFiles>")
    set(have_FI ON)
  endif()
endforeach()

if (NOT have_FI)
  set(RunCMake_TEST_FAILED "Generated project does not have expected ForcedIncludeFiles.")
  return()
endif()
