define_property(TARGET PROPERTY PROP2
  INITIALIZE_FROM_VARIABLE Test_PROP2
  )
define_property(TARGET PROPERTY PROP3
  INITIALIZE_FROM_VARIABLE Test_PROP3
  )

add_executable(sub_exe ../main.c)
assert_prop_eq(sub_exe PROP1 "Hello")
assert_prop_eq(sub_exe PROP2 "world")
assert_prop_eq(sub_exe PROP3 "!")
