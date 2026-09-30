include("${CMAKE_CURRENT_LIST_DIR}/exp-compile-commands-check.cmake")

set(imported_module_commands 0)
foreach (item RANGE "${length}")
  string(JSON source GET "${compile_commands}" "${item}" "file")
  string(JSON output GET "${compile_commands}" "${item}" "output")
  if (source MATCHES "/exp-iface-build/(subdir/)?importable\\.cxx$" AND
      output MATCHES "@synth_")
    math(EXPR imported_module_commands "${imported_module_commands} + 1")
  endif ()
endforeach ()

if (imported_module_commands LESS 2)
  list(APPEND RunCMake_TEST_FAILED
    "Missing compile commands for imported module synthetic targets.")
endif ()
