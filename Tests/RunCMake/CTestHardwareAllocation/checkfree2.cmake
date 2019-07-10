setup_hardware_tests()

# This test is an attack on the hardware scheduling algorithm. It has been
# carefully crafted to fool the algorithm into thinking there isn't sufficient
# hardware for it.
add_hardware_test(Test1 1 "widgets:2;4,widgets:4")

cleanup_hardware_tests()
