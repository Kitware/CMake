enable_language(C)

file(WRITE "${CMAKE_BINARY_DIR}/test.c" "__declspec(dllexport) void test(void) {}\n")
file(WRITE "${CMAKE_BINARY_DIR}/main.c" [[__declspec(dllimport) extern void test(void);

int main(void)
{
  test();
  return 0;
}
]])

add_library(test SHARED "${CMAKE_BINARY_DIR}/test.c")
add_executable(exe "${CMAKE_BINARY_DIR}/main.c")
target_link_libraries(exe PRIVATE test)

install(TARGETS exe DESTINATION bin)

install(CODE [[
  file(MAKE_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:test>")
  file(GET_RUNTIME_DEPENDENCIES
    EXECUTABLES
      "${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:exe>"
    PRE_INCLUDE_REGEXES "^(lib)?test\\.dll$"
    PRE_EXCLUDE_REGEXES ".*"
    )
  message(FATAL_ERROR "This message should not be displayed")
  ]])
