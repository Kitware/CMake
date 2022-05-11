enable_language(C)

function(assert_prop_eq tgt name value)
  get_property(actual_value TARGET ${tgt} PROPERTY ${name})
  if(NOT actual_value STREQUAL value)
    message(SEND_ERROR "Expected value of ${name}:\n  ${value}\nActual value:\n  ${actual_value}")
  endif()
endfunction()

function(assert_prop_undef tgt name)
  get_property(actual_value TARGET ${tgt} PROPERTY ${name})
  if(DEFINED actual_value)
    message(SEND_ERROR "Expected ${name} to be undefined, actual value:\n  ${actual_value}")
  endif()
endfunction()

set(Test_PROP1 "Hello")
set(Test_PROP2 "world")
set(MyTest_PROP3 "!")
define_property(TARGET PROPERTY Test_PROP1
  INITIALIZE_FROM_VARIABLE Test_PROP1
  )

add_subdirectory(define_property-INITIALIZE_FROM_VARIABLE-subdirectory)

add_executable(top_exe main.c)
assert_prop_eq(top_exe Test_PROP1 "Hello")
assert_prop_eq(top_exe Test_PROP2 "world")
assert_prop_eq(top_exe Test_PROP3 "!")
