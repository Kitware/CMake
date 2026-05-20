include("${CMAKE_CURRENT_LIST_DIR}/CMP0219-helpers.cmake")

# Build progressively escaped variants to model callers that added extra
# backslashes to make an OLD-only macro chain work.
string(REPLACE "\\" "\\\\" cmp0219_path_2 "${cmp0219_path_native}")
string(REPLACE "\\" "\\\\" cmp0219_path_4 "${cmp0219_path_2}")
string(REPLACE "\\" "\\\\" cmp0219_path_8 "${cmp0219_path_4}")

cmake_policy(SET CMP0219 OLD)

macro(cmp0219_leaf var_name)
  set(${var_name} "${ARGN}")
endmacro()

macro(cmp0219_middle_old)
  cmp0219_leaf(cmp0219_old_chain_capture ${ARGN})
endmacro()

macro(cmp0219_middle_new)
  # Simulate a dependency update opting this middle forwarding layer into NEW.
  cmake_policy(PUSH)
  cmake_policy(SET CMP0219 NEW)
  cmp0219_leaf(cmp0219_mixed_chain_capture ${ARGN})
  cmake_policy(POP)
endmacro()

macro(cmp0219_outer_to_old)
  cmp0219_middle_old(${ARGN})
endmacro()

macro(cmp0219_outer_to_new_middle)
  cmp0219_middle_new(${ARGN})
endmacro()

# OLD->OLD->OLD requires heavily escaped input and produces a native path.
cmp0219_outer_to_old(HINTS "${cmp0219_path_8}")
cmp0219_assert_equal(
  "${cmp0219_old_chain_capture}" "HINTS;${cmp0219_path_native}")

# OLD->NEW->OLD with the same input preserves one extra layer.
cmp0219_outer_to_new_middle(HINTS "${cmp0219_path_8}")
cmp0219_assert_equal(
  "${cmp0219_mixed_chain_capture}" "HINTS;${cmp0219_path_2}")

# OLD->NEW->OLD needs fewer escape layers to reach a native path.
cmp0219_outer_to_new_middle(HINTS "${cmp0219_path_4}")
cmp0219_assert_equal(
  "${cmp0219_mixed_chain_capture}" "HINTS;${cmp0219_path_native}")
