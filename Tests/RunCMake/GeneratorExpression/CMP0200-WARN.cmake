project(test-CMP0200-WARN C)

set(CMAKE_POLICY_WARNING_CMP0200 ON)

add_library(lib_test1 INTERFACE IMPORTED)
set_target_properties(lib_test1 PROPERTIES
  IMPORTED_CONFIGURATIONS "DOG;CAT"
)

add_executable(exe_test1 configtest.c)
target_link_libraries(exe_test1 PRIVATE lib_test1)

add_library(lib_test2 INTERFACE IMPORTED)

add_executable(exe_test2 configtest.c)
target_link_libraries(exe_test2 PRIVATE lib_test2)
