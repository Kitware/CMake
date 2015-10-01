set(CMAKE_BUILD_WITH_INSTALL_RPATH 1)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/test_lib.hpp"
    "int test_lib();\n")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/test_lib.cpp"
    "#include \"test_lib.hpp\"\nint test_lib() {return 0;}\n")
add_library(test_lib SHARED "${CMAKE_CURRENT_BINARY_DIR}/test_lib.cpp")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/main.cpp"
    "#include \"test_lib.hpp\"\nint main() {return test_lib();}\n")
add_executable(test_prog "${CMAKE_CURRENT_BINARY_DIR}/main.cpp")
target_link_libraries(test_prog test_lib)

install(TARGETS test_prog DESTINATION foo COMPONENT applications)
install(TARGETS test_prog DESTINATION foo_auto COMPONENT applications_auto)
install(FILES CMakeLists.txt DESTINATION bar COMPONENT headers)
install(TARGETS test_lib DESTINATION bas COMPONENT libs)
install(TARGETS test_lib DESTINATION bas_auto COMPONENT libs_auto)

set(CPACK_PACKAGE_NAME "dependencies")
