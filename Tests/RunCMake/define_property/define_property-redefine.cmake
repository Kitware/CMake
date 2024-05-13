function(verify_value type prop attrib expected actual)
  if(expected STREQUAL "FALSE")
    if(actual)
      message(FATAL_ERROR
        "Expected ${type} property ${prop}'s ${attrib} to be false")
    endif()
  elseif(expected STREQUAL "TRUE")
    if(NOT actual)
      message(FATAL_ERROR
        "Expected ${type} property ${prop}'s ${attrib} to be true")
    endif()
  elseif(NOT actual STREQUAL expected)
    message(FATAL_ERROR
      "Expected value of ${type} property ${prop}'s ${attrib}:\n"
      "  ${expected}\n"
      "Actual value:\n"
      "  ${actual}"
    )
  endif()
endfunction()

function(assert_tgt_prop_attrib_eq prop attrib expected)
  get_property(actual TARGET NONE PROPERTY "${prop}" "${attrib}")
  verify_value(TARGET "${prop}" "${attrib}" "${expected}" "${actual}")
endfunction()

function(assert_dir_prop_attrib_eq prop attrib expected)
  get_property(actual DIRECTORY "" PROPERTY "${prop}" "${attrib}")
  verify_value(DIRECTORY "${prop}" "${attrib}" "${expected}" "${actual}")
endfunction()

#
# TESTS
#

# Define a new target property
message(CHECK_START "Testing define_property(TARGET ...)")
define_property(TARGET PROPERTY TGT1
  BRIEF_DOCS "Brief")
assert_tgt_prop_attrib_eq(TGT1 BRIEF_DOCS "Brief")
assert_tgt_prop_attrib_eq(TGT1 FULL_DOCS "NOTFOUND")
message(CHECK_PASS "Complete")

# Attempt to redefine with different/additional attributes
message(CHECK_START "Testing TARGET property redefinition")
define_property(TARGET PROPERTY TGT1
  BRIEF_DOCS "Changed"
  FULL_DOCS "Full")
assert_tgt_prop_attrib_eq(TGT1 BRIEF_DOCS "Brief")
assert_tgt_prop_attrib_eq(TGT1 FULL_DOCS "NOTFOUND")
message(CHECK_PASS "Complete")

# Query undefined property
message(CHECK_START "Testing undefined TARGET property query")
assert_tgt_prop_attrib_eq(TGT2 DEFINED FALSE)
assert_tgt_prop_attrib_eq(TGT2 BRIEF_DOCS "NOTFOUND")
message(CHECK_PASS "Complete")

# Define after query
message(CHECK_START "Testing TARGET property definition after query")
define_property(TARGET PROPERTY TGT2
  BRIEF_DOCS "Brief"
  FULL_DOCS "Full"
)
assert_tgt_prop_attrib_eq(TGT2 DEFINED TRUE)
assert_tgt_prop_attrib_eq(TGT2 BRIEF_DOCS "Brief")
assert_tgt_prop_attrib_eq(TGT2 FULL_DOCS "Full")
message(CHECK_PASS "Complete")

# Define a new directory property
message(CHECK_START "Testing define_property(DIRECTORY ...)")
define_property(DIRECTORY PROPERTY DIR1
  BRIEF_DOCS "Brief"
  FULL_DOCS "Full"
)
assert_dir_prop_attrib_eq(DIR1 DEFINED TRUE)
assert_dir_prop_attrib_eq(DIR1 BRIEF_DOCS "Brief")
assert_dir_prop_attrib_eq(DIR1 FULL_DOCS "Full")
message(CHECK_PASS "Complete")

# Attempt to redefine existing attributes
message(CHECK_START "Testing DIRECTORY property redefinition")
define_property(DIRECTORY PROPERTY DIR1
  BRIEF_DOCS "Overwritten"
)
assert_dir_prop_attrib_eq(DIR1 BRIEF_DOCS "Brief")
assert_dir_prop_attrib_eq(DIR1 FULL_DOCS "Full")
message(CHECK_PASS "Complete")
