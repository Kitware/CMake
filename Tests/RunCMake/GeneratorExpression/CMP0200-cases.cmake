# Under CMP0200 OLD, CMake fails to select a valid configuration for an
# imported INTERFACE library with no location, and will (as an implementation
# artifact) select the last configuration in IMPORTED_CONFIGURATIONS.

# Under NEW, CMake should select a configuration which matches the current
# build type, if available in IMPORTED_CONFIGURATIONS.
function(do_match_config_test)
  add_library(lib_match INTERFACE IMPORTED)
  set_target_properties(lib_match PROPERTIES
    IMPORTED_CONFIGURATIONS "TEST;RELEASE;DEBUG"
    INTERFACE_COMPILE_DEFINITIONS
      "$<$<CONFIG:debug>:DEBUG>;$<$<CONFIG:release>:RELEASE>;$<$<CONFIG:test>:TEST>"
  )

  add_executable(exe_match configtest.c)
  target_compile_definitions(exe_match PRIVATE ${ARGN})
  target_link_libraries(exe_match PRIVATE lib_match)
endfunction()

# Under NEW, CMake should select the first of IMPORTED_CONFIGURATIONS if no
# mapping exists and no configuration matching the current build type exists.
function(do_first_config_test)
  add_library(lib_first INTERFACE IMPORTED)
  set_target_properties(lib_first PROPERTIES
    IMPORTED_CONFIGURATIONS "TEST;DEBUG"
    INTERFACE_COMPILE_DEFINITIONS
      "$<$<CONFIG:debug>:DEBUG>;$<$<CONFIG:test>:TEST>"
  )

  add_executable(exe_first configtest.c)
  target_compile_definitions(exe_first PRIVATE ${ARGN})
  target_link_libraries(exe_first PRIVATE lib_first)
endfunction()
