enable_language(C)
if(CMake_TEST_Swift)
  enable_language(Swift)
  string(APPEND CMAKE_Swift_FLAGS " -DSWIFTFLAG")
  add_executable(swift_inherit_test dummy_main.swift)
endif()

add_compile_definitions(TEST_INHERITTEST)
string(APPEND CMAKE_C_FLAGS " -DTESTFLAG=\\\"TEST_INHERITTEST\\\"")

add_executable(inherit_test main.c)

target_link_libraries(inherit_test PRIVATE "TEST_INHERITTEST")
