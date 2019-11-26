set(vcProjectFile "${RunCMake_TEST_BINARY_DIR}/foo.vcxproj")
if(NOT EXISTS "${vcProjectFile}")
  set(RunCMake_TEST_FAILED "Project file foo.vcxproj does not exist.")
  return()
endif()


set(test1Library "boost")
set(test1Version "1.7.0")


set(test2Library "SFML")
set(test2Version "2.2.0")

set(Library1Found FALSE)
set(Library2Found FALSE)

file(STRINGS "${vcProjectFile}" lines)

foreach(i 1 2)
  set(testLibrary "${test${i}Library}")
  set(testVersion "${test${i}Version}")
  foreach(line IN LISTS lines)
    if(line MATCHES "^ *<PackageReference Include=\"${testLibrary}\".*>$")
      if(line MATCHES "^ *<PackageReference .* Version=\"${testVersion}\".*>$")
        set(Library${i}Found TRUE)
        message(STATUS "foo.vcxproj is using package reference ${testLibrary} with version ${testVersion}")
      else()
        message(STATUS "foo.vcxproj failed to define reference ${testLibrary} with version ${testVersion}")
        set(Library${i}Found FALSE)
      endif()
    endif()
  endforeach()
endforeach()

if(NOT Library1Found OR NOT Library2Found)
  set(RunCMake_TEST_FAILED "Failed to find package references")
  return()
endif()
