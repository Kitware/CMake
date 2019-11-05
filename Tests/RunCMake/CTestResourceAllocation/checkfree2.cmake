setup_resource_tests()

# This test is an attack on the resource scheduling algorithm. It has been
# carefully crafted to fool the algorithm into thinking there aren't sufficient
# resources for it.
add_resource_test(Test1 1 "widgets:2;4,widgets:4")

cleanup_resource_tests()
