set(CMAKE_VERBOSE_MAKEFILE TRUE)

# Use the noop genexp $<PATH:...> genexp to validate genexp support.
set_property(GLOBAL PROPERTY
  CMAKE_COMPILE_WRAPPER_EXAMPLE_1
    "$<PATH:CMAKE_PATH,${CMAKE_COMMAND}>" -E env CW_OUTER=1
)
set_property(GLOBAL PROPERTY
  CMAKE_COMPILE_WRAPPER_EXAMPLE_2
    "${CMAKE_COMMAND}" -E env CW_INNER=2
)
set_property(GLOBAL PROPERTY
  CMAKE_LINK_WRAPPER_EXAMPLE_1
    "${CMAKE_COMMAND}" -E env LW_OUTER=1
)
set_property(GLOBAL PROPERTY
  CMAKE_LINK_WRAPPER_EXAMPLE_2
    "${CMAKE_COMMAND}" -E env LW_INNER=2
)

add_library(iface INTERFACE)
set_target_properties(iface PROPERTIES
  INTERFACE_COMPILE_WRAPPERS EXAMPLE_1
  INTERFACE_LINK_WRAPPERS EXAMPLE_1
)

function(check_wrapper_property target prop expected)
  get_target_property(actual "${target}" "${prop}")
  if(NOT actual STREQUAL expected)
    message(FATAL_ERROR
      "get_target_property(${target} ${prop}) returned [${actual}], "
      "expected [${expected}]")
  endif()
endfunction()

check_wrapper_property(iface INTERFACE_COMPILE_WRAPPERS "EXAMPLE_1")
check_wrapper_property(iface INTERFACE_LINK_WRAPPERS "EXAMPLE_1")

function(example_exe)
  # ensure no temp file will be used
  get_property(langs GLOBAL PROPERTY ENABLED_LANGUAGES)
  foreach(lang IN LISTS langs)
    foreach(rule IN ITEMS COMPILE_OBJECT LINK_EXECUTABLE)
      set(var CMAKE_${lang}_${rule})
      string(REPLACE "${CMAKE_START_TEMP_FILE}" "" ${var} "${${var}}")
      string(REPLACE "${CMAKE_END_TEMP_FILE}" "" ${var} "${${var}}")
      set(${var} "${${var}}" PARENT_SCOPE)
    endforeach()
  endforeach()

  add_executable(main ${ARGN})
  target_link_libraries(main PRIVATE iface)

  # We repeat options from INTERFACE_* here, which CMake should de-duplicate.
  set(compile_wrappers "EXAMPLE_1;EXAMPLE_2")
  set(link_wrappers "EXAMPLE_2")

  set_target_properties(main PROPERTIES
    COMPILE_WRAPPERS "${compile_wrappers}"
    LINK_WRAPPERS "${link_wrappers}"
  )

  # The properties must be readable back exactly as set.
  check_wrapper_property(main COMPILE_WRAPPERS "${compile_wrappers}")
  check_wrapper_property(main LINK_WRAPPERS "${link_wrappers}")
endfunction()
