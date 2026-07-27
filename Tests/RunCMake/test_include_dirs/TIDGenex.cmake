enable_testing()

set(gen_dir "${CMAKE_CURRENT_BINARY_DIR}")

# Plain include script (no generator expression in the path); used via the
# deprecated singular TEST_INCLUDE_FILE to also cover singular-first ordering.
file(GENERATE OUTPUT "${gen_dir}/plain.cmake"
  CONTENT "add_test(plain_test \"${CMAKE_COMMAND}\" -E true)\n")

# Per-configuration script selected by $<CONFIG> in the include path.
file(GENERATE OUTPUT "${gen_dir}/props_$<CONFIG>.cmake"
  CONTENT "add_test(config_$<CONFIG> \"${CMAKE_COMMAND}\" -E true)\n")

# Config-independent generator expression in the path: collapses to a single
# unconditional include (still runs without -C).
file(GENERATE OUTPUT "${gen_dir}/indep.cmake"
  CONTENT "add_test(indep_test \"${CMAKE_COMMAND}\" -E true)\n")

# Debug-only include via a conditional-empty generator expression.
file(GENERATE OUTPUT "${gen_dir}/dbgonly.cmake"
  CONTENT "add_test(dbgonly_test \"${CMAKE_COMMAND}\" -E true)\n")

set_property(DIRECTORY PROPERTY TEST_INCLUDE_FILE
  "${gen_dir}/plain.cmake")
set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES
  "${gen_dir}/props_$<CONFIG>.cmake")
set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES
  "$<1:${gen_dir}/indep.cmake>")
set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES
  "$<$<CONFIG:Debug>:${gen_dir}/dbgonly.cmake>")
