set(_simpletest_impl_script ${CMAKE_CURRENT_LIST_DIR}/simpletest_discover_impl.cmake)

function(simpletest_discover_tests target)
  if(NOT TARGET ${target})
    message(FATAL_ERROR "simpletest_discover_tests: no such target '${target}'")
  endif()

  set(_out ${CMAKE_CURRENT_BINARY_DIR}/${target}_ctests.cmake)

  if(NOT EXISTS ${_out})
    file(WRITE ${_out} "# Populated after building ${target}\n")
  endif()

# noqa: spellcheck off
  add_custom_command(TARGET ${target} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
      -DTEST_EXE=$<TARGET_FILE:${target}>
      -DOUT_FILE=${_out}
      -P ${_simpletest_impl_script}
    BYPRODUCTS ${_out}
    COMMENT "SimpleTest: Discovering tests in ${target}"
    VERBATIM
  )
# noqa: spellcheck on

  set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES ${_out})
endfunction()
