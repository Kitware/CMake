include(CTest)
add_test(NAME "unreachable" COMMAND ${CMAKE_COMMAND} -E true)
set_property(DIRECTORY PROPERTY TEST_INCLUDE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/TestfileErrors-Script.cmake)
