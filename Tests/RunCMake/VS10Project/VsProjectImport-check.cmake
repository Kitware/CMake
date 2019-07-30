set(vcProjectFile "${RunCMake_TEST_BINARY_DIR}/foo.vcxproj")
if(NOT EXISTS "${vcProjectFile}")
  set(RunCMake_TEST_FAILED "Project file ${vcProjectFile} does not exist.")
  return()
endif()

set(test1Import "path\\\\to\\\\nuget_packages\\\\Foo.1.0.0\\\\build\\\\Foo.props")
set(test2Import "path\\\\to\\\\nuget_packages\\\\Bar.1.0.0\\\\build\\\\Bar.props")

set(import1Found FALSE)
set(import2Found FALSE)

file(STRINGS "${vcProjectFile}" lines)

foreach(i 1 2)
  set(testImport "${test${i}Import}")
  foreach(line IN LISTS lines)
    if(line MATCHES "^ *<Import Project=\".*${test1Import}\" />$")
      message(STATUS "foo.vcxproj is using project import ${testImport}")
      set(import${i}Found TRUE)
    endif()
  endforeach()
endforeach()

if(NOT import1Found OR NOT import2Found)
  set(RunCMake_TEST_FAILED "Imported project not found.")
  return()
endif()
