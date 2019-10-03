setup_hardware_tests()

add_hardware_test(Test1 4 "transmogrifiers:2")

# Mitigate possible race conditions to ensure that the events are logged in the
# exact order we want
add_test(NAME Test2Sleep COMMAND "${CMAKE_COMMAND}" -E sleep 2)
add_hardware_test(Test2 4 "transmogrifiers:2")
set_property(TEST Test2 APPEND PROPERTY DEPENDS Test2Sleep)

cleanup_hardware_tests()
