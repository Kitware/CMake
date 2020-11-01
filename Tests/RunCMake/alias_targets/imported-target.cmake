
enable_language(CXX)


add_executable(test-exe IMPORTED)
add_executable(alias-test-exe ALIAS test-exe)

if(TARGET alias-test-exe)
  get_target_property(aliased-target alias-test-exe ALIASED_TARGET)
  if("${aliased-target}" STREQUAL "test-exe")
    get_target_property(aliased-name alias-test-exe NAME)
    if("${aliased-name}" STREQUAL "test-exe")
      message("'alias-test-exe' is an alias for '${aliased-target}'"
              " and its name-property contains '${aliased-name}'.")
    else()
      message("'alias-test-exe' is an alias for '${aliased-target}'"
              " but its name-property contains '${aliased-name}'!?")
    endif()
  else()
    message("'alias-test-exe' is something but not a real target!?")
  endif()
else()
    message("'alias-test-exe' does not exist!?")
endif()


add_library(test-lib SHARED IMPORTED)
add_library(alias-test-lib ALIAS test-lib)

if(TARGET alias-test-lib)
  get_target_property(aliased-target alias-test-lib ALIASED_TARGET)
  if("${aliased-target}" STREQUAL "test-lib")
    get_target_property(aliased-name alias-test-lib NAME)
    if("${aliased-name}" STREQUAL "test-lib")
      message("'alias-test-lib' is an alias for '${aliased-target}'"
              " and its name-property contains '${aliased-name}'.")
    else()
      message("'alias-test-lib' is an alias for '${aliased-target}'"
              " but its name-property contains '${aliased-name}'!?")
    endif()
  else()
    message("'alias-test-lib' is something but not a real target!?")
  endif()
else()
    message("'alias-test-lib' does not exist!?")
endif()

add_subdirectory (imported-target-subdir1)
add_subdirectory (imported-target-subdir2)

foreach (alias IN ITEMS exe-local lib-local
                        exe-subdir1 lib-subdir1
                        exe-subdir2 lib-subdir2)
  if (TARGET alias-test-${alias})
    message (SEND_ERROR "ALIAS 'alias-test-${alias}' unexpectedly defined.")
  endif()
endforeach()
