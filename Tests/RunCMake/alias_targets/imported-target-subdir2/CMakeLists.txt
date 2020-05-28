
add_executable(alias-test-exe-subdir2 ALIAS test-exe)
add_executable(alias-test-exe-local ALIAS test-exe)

add_library(alias-test-lib-subdir2 ALIAS test-lib)
add_library(alias-test-lib-local ALIAS test-lib)


foreach (item IN ITEMS exe lib)
  get_property (aliasedTarget TARGET alias-test-${item}-local PROPERTY ALIASED_TARGET)
  if (NOT aliasedTarget STREQUAL "test-${item}")
    message (SEND_ERROR "Wrong aliased target '${aliasedTarget}' for ALIAS 'alias-test-${item}-local'.")
  endif()
endforeach()

foreach (item IN ITEMS exe lib)
  if (TARGET alias-test-${item}-subdir1)
    message (SEND_ERROR "ALIAS 'alias-test-${item}-subdir1' unexpectedly defined.")
  endif()
endforeach()
