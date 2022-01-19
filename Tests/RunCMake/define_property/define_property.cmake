function(assert_prop_scope_eq prop scope value)
  get_property(actual_value TARGET NONE PROPERTY ${prop} ${scope})
  if(NOT actual_value STREQUAL value)
    message(SEND_ERROR "Expected value of ${prop}'s ${scope}:\n  ${value}\nActual value:\n  ${actual_value}")
  endif()
endfunction()

define_property(TARGET PROPERTY PROP1)
define_property(TARGET PROPERTY PROP2
  BRIEF_DOCS "Brief")
define_property(TARGET PROPERTY PROP3
  FULL_DOCS "Full")
define_property(TARGET PROPERTY PROP4
  BRIEF_DOCS "Brief"
  FULL_DOCS "Full")

assert_prop_scope_eq(PROP0 BRIEF_DOCS "NOTFOUND")
assert_prop_scope_eq(PROP0 FULL_DOCS "NOTFOUND")
assert_prop_scope_eq(PROP1 BRIEF_DOCS "NOTFOUND")
assert_prop_scope_eq(PROP1 FULL_DOCS "NOTFOUND")
assert_prop_scope_eq(PROP2 BRIEF_DOCS "Brief")
assert_prop_scope_eq(PROP2 FULL_DOCS "NOTFOUND")
assert_prop_scope_eq(PROP3 BRIEF_DOCS "NOTFOUND")
assert_prop_scope_eq(PROP3 FULL_DOCS "Full")
assert_prop_scope_eq(PROP4 BRIEF_DOCS "Brief")
assert_prop_scope_eq(PROP4 FULL_DOCS "Full")
