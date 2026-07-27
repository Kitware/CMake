enable_testing()

# An unknown generator expression in the include path must fail at generate
# time (diagnostics are best-effort: they need not name the set_property call).
set_property(DIRECTORY PROPERTY TEST_INCLUDE_FILE
  "${CMAKE_CURRENT_BINARY_DIR}/x_$<NOTAGENEX>.cmake")
