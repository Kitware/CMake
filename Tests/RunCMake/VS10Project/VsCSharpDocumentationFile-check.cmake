#
# Check C# VS project for required elements
#
set(csProjectFile "${RunCMake_TEST_BINARY_DIR}/foo.csproj")
if(NOT EXISTS "${csProjectFile}")
  set(RunCMake_TEST_FAILED "Project file ${csProjectFile} does not exist.")
  return()
endif()

file(STRINGS "${csProjectFile}" lines)

set(HAVE_DocumentationFile 0)
foreach(line IN LISTS lines)
  if(line MATCHES "^ *<DocumentationFile>([^<>]+)</DocumentationFile>")
    if(HAVE_DocumentationFile)
      set(RunCMake_TEST_FAILED "Documentation node has been generated more than once for\n  ${csProjectFile}")
      return()
    endif()
    set(HAVE_DocumentationFile 1)
  endif()
endforeach()

if(NOT HAVE_DocumentationFile)
  set(RunCMake_TEST_FAILED "Documentation node has not been generated for\n  ${csProjectFile}")
  return()
endif()
