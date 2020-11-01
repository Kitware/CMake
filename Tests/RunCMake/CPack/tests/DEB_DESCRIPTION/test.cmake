install(FILES CMakeLists.txt DESTINATION satu COMPONENT satu)
install(FILES CMakeLists.txt DESTINATION dua COMPONENT dua)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "This is the summary line")
set(_description [[This is the Debian package multiline description.

It must be formatted properly! Otherwise, the result `*.deb`
package become broken and cannot be installed!

It may contains `;` characters (even like this `;;;;`). Example:

  - one;
  - two;
  - three;

... and they are properly handled by the automatic description formatter!

See also: https://www.debian.org/doc/debian-policy/ch-controlfields.html#description]])

if(RunCMake_SUBTEST_SUFFIX STREQUAL "CPACK_DEBIAN_PACKAGE_DESCRIPTION")
  if(PACKAGING_TYPE STREQUAL "COMPONENT")
    set(CPACK_DEBIAN_SATU_DESCRIPTION "${_description}")
    set(CPACK_DEBIAN_DUA_DESCRIPTION "${_description}")
  else()
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${_description}")
  endif()

elseif(RunCMake_SUBTEST_SUFFIX STREQUAL "CPACK_PACKAGE_DESCRIPTION")
  # NOTE Documented fallback variable
  if(PACKAGING_TYPE STREQUAL "COMPONENT")
    set(CPACK_COMPONENT_SATU_DESCRIPTION "${_description}")
    set(CPACK_COMPONENT_DUA_DESCRIPTION "${_description}")
  else()
    set(CPACK_PACKAGE_DESCRIPTION "${_description}")
  endif()

elseif(RunCMake_SUBTEST_SUFFIX STREQUAL "CPACK_COMPONENT_COMP_DESCRIPTION")
  # NOTE Documented fallback variable without CPACK_PACKAGE_DESCRIPTION_SUMMARY
  if(PACKAGING_TYPE STREQUAL "COMPONENT")
    set(CPACK_COMPONENT_SATU_DESCRIPTION "One line description")
    set(CPACK_COMPONENT_DUA_DESCRIPTION "One line description")
  else()
    set(CPACK_PACKAGE_DESCRIPTION "One line description")
  endif()
  unset(CPACK_PACKAGE_DESCRIPTION_SUMMARY)

elseif(RunCMake_SUBTEST_SUFFIX STREQUAL "CPACK_PACKAGE_DESCRIPTION_FILE")
  # NOTE Getting the description from the file
  set(_file "${CMAKE_CURRENT_BINARY_DIR}/description.txt")
  file(WRITE "${_file}" "${_description}")
  set(CPACK_PACKAGE_DESCRIPTION_FILE "${_file}")

endif()

# kate: indent-width 2;
