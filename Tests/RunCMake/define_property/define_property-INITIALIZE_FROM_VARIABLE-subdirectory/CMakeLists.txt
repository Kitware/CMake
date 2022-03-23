define_property(TARGET PROPERTY Test_PROP2
  INITIALIZE_FROM_VARIABLE Test_PROP2
  )
define_property(TARGET PROPERTY Test_PROP3
  INITIALIZE_FROM_VARIABLE MyTest_PROP3
  )

add_executable(sub_exe ../main.c)
assert_prop_eq(sub_exe Test_PROP1 "Hello")
assert_prop_eq(sub_exe Test_PROP2 "world")
assert_prop_eq(sub_exe Test_PROP3 "!")
