enable_language(C)

set(CMAKE_LINKER_TYPE APPLE_CLASSIC)

add_executable(main main.c)
target_link_libraries(main PRIVATE m m)

if(CMake_TEST_Swift)
  enable_language(Swift)
  add_executable(main_swift main.swift)
  target_link_libraries(main_swift PRIVATE m m)
endif()
