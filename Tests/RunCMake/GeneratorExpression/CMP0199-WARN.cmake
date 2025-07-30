project(test-CMP0199-WARN C)

set(CMAKE_POLICY_WARNING_CMP0199 ON)

add_library(lib_test INTERFACE IMPORTED)
set_target_properties(lib_test PROPERTIES
  INTERFACE_COMPILE_DEFINITIONS "$<$<CONFIG:release>:RELEASE>"
)

add_executable(exe_test configtest.c)
target_link_libraries(exe_test PRIVATE lib_test)
