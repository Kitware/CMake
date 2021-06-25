
include(CTest)
enable_testing()

add_test(NAME dummy COMMAND ${CMAKE_COMMAND} -E echo $<LINK_LANG_AND_ID:CXX,GNU>)
