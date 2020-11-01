setup_resource_tests()

add_resource_test(Test1 1 "widgets:8")
add_resource_test(Test2 1 "fluxcapacitors:50;fluxcapacitors:50,widgets:8")
add_resource_test(Test3 1 "fluxcapacitors:121")

cleanup_resource_tests()
