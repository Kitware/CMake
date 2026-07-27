enable_testing()

set(d "${CMAKE_CURRENT_BINARY_DIR}")

# This case only asserts serialization of the generated CTestTestfile.cmake;
# ctest is never run, so the referenced paths need not exist.

# (1) A genex result containing ';' is split into multiple includes (list
#     semantics, as for usage-requirement properties).  $<SEMICOLON> keeps the
#     ';' out of the pre-eval list split so it appears only in the evaluated
#     result, which is then re-split into one include() per element.
set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES
  "$<1:${d}/a$<SEMICOLON>b.cmake>")

# (2) A genex result containing a space stays double-quoted.
set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES
  "$<1:${d}/a b.cmake>")

# (3) A genex result containing '$' is forced to bracket form so CTest does not
#     perform a second-stage ${...} expansion.  $<1:$> composes a literal '$'.
set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES
  "$<1:${d}/x$<1:$>y.cmake>")

# (4) A plain entry containing ${VAR} stays a raw, bare double-quoted include so
#     CTest still expands it at test time (must NOT be routed through Quote).
set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES
  "${d}/lit_\${MY_VAR}.cmake")
