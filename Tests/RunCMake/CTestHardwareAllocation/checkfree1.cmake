setup_hardware_tests()

add_hardware_test(Test1 1 "widgets:8")
add_hardware_test(Test2 1 "fluxcapacitors:50;fluxcapacitors:50,widgets:8")
add_hardware_test(Test3 1 "fluxcapacitors:121")

cleanup_hardware_tests()
