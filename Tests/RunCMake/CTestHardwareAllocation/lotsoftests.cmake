setup_hardware_tests()

add_hardware_test(Test1 2 "widgets:8;2,widgets:2")
add_hardware_test(Test2 5 "fluxcapacitors:40")
add_hardware_test(Test3 1 "10,widgets:1,fluxcapacitors:2")
add_hardware_test(Test4 4 "fluxcapacitors:121")

foreach(i RANGE 5 50)
  add_hardware_test(Test${i} 1 "2,widgets:1")
endforeach()

foreach(i RANGE 51 100)
  add_hardware_test(Test${i} 1 "2,transmogrifiers:2")
endforeach()

cleanup_hardware_tests()
