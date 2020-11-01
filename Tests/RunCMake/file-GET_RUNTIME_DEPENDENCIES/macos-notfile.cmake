enable_language(C)

file(WRITE "${CMAKE_BINARY_DIR}/test.c" "void test(void) {}\n")
file(WRITE "${CMAKE_BINARY_DIR}/main.c" [[extern void test(void);

int main(void)
{
  test();
  return 0;
}
]])

add_library(test SHARED "${CMAKE_BINARY_DIR}/test.c")
set_property(TARGET test PROPERTY INSTALL_NAME_DIR @rpath)
add_executable(exe "${CMAKE_BINARY_DIR}/main.c")
target_link_libraries(exe PRIVATE test)
set_property(TARGET exe PROPERTY INSTALL_RPATH "@loader_path/../lib")

install(TARGETS exe DESTINATION bin)

install(CODE [[
  file(MAKE_DIRECTORY "${CMAKE_INSTALL_PREFIX}/lib/$<TARGET_FILE_NAME:test>")
  file(GET_RUNTIME_DEPENDENCIES
    EXECUTABLES
      "${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:exe>"
    PRE_INCLUDE_REGEXES "^@rpath/libtest\\.dylib$"
    PRE_EXCLUDE_REGEXES ".*"
    )
  message(FATAL_ERROR "This message should not be displayed")
  ]])
