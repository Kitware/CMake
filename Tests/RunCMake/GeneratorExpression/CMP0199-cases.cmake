# Under CMP0199 OLD, $<CONFIG> matches not just the selected configuration, but
# every entry in MAP_IMPORTED_CONFIG_<CONFIG>. Under NEW, it should only match
# the selected configuration.
function(do_mapped_config_test)
  add_library(lib_mapped INTERFACE IMPORTED)
  set_target_properties(lib_mapped PROPERTIES
    IMPORTED_CONFIGURATIONS "TEST"
    INTERFACE_COMPILE_DEFINITIONS
      "$<$<CONFIG:debug>:DEBUG>;$<$<CONFIG:release>:RELEASE>;$<$<CONFIG:test>:TEST>"
    MAP_IMPORTED_CONFIG_RELEASE "RELEASE;DEBUG;TEST"
  )

  add_executable(exe_mapped configtest.c)
  target_compile_definitions(exe_mapped PRIVATE ${ARGN})
  target_link_libraries(exe_mapped PRIVATE lib_mapped)
endfunction()

# Under CMake CMP0199 OLD, $<CONFIG> matches both the consumer's configuration
# AND the selected configuration of the library being consumed. Under NEW, it
# should only match the selected configuration.
function(do_unique_config_test)
  add_library(lib_unique INTERFACE IMPORTED)
  set_target_properties(lib_unique PROPERTIES
    IMPORTED_CONFIGURATIONS "DEBUG"
    INTERFACE_COMPILE_DEFINITIONS
      "$<$<CONFIG:debug>:DEBUG>;$<$<CONFIG:release>:RELEASE>"
  )

  add_executable(exe_unique configtest.c)
  target_compile_definitions(exe_unique PRIVATE ${ARGN})
  target_link_libraries(exe_unique PRIVATE lib_unique)
endfunction()
