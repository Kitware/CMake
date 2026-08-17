# Fixture C is required by a test of fixture A and by a test of fixture B, so
# all three repeat as one unit.  The DEPENDS chain fixes the order in which
# the unit runs its tests.
enable_testing()
cmake_policy(SET CMP0224 NEW)

foreach(f A B C)
  add_test(NAME setup${f} COMMAND ${CMAKE_COMMAND} -E true)
  set_tests_properties(setup${f} PROPERTIES FIXTURES_SETUP ${f})
endforeach()
set_tests_properties(setupB PROPERTIES DEPENDS setupA)
set_tests_properties(setupC PROPERTIES DEPENDS setupB)

add_test(NAME testA COMMAND ${CMAKE_COMMAND} -E true)
set_tests_properties(testA PROPERTIES FIXTURES_REQUIRED "A;C")

add_test(NAME testB COMMAND ${CMAKE_COMMAND} -E true)
set_tests_properties(testB PROPERTIES FIXTURES_REQUIRED "B;C" DEPENDS testA)
