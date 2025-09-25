include(CTest)
add_test(NAME "unreachable" COMMAND ${CMAKE_COMMAND} -E true)
set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/TestfileErrors-Script.cmake")
