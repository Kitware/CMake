enable_language(C)

file(WRITE "${CMAKE_BINARY_DIR}/test.c" "__declspec(dllexport) void test(void) {}\n")
file(WRITE "${CMAKE_BINARY_DIR}/main.c" [[__declspec(dllimport) extern void test(void);

int main(void)
{
  test();
  return 0;
}
]])

add_subdirectory(CMP0207-subdir)

add_executable(exe "${CMAKE_BINARY_DIR}/main.c")
target_link_libraries(exe PRIVATE test)

install(TARGETS test DESTINATION bin/lib1)

install(
  TARGETS exe
  DESTINATION results_old
  RUNTIME_DEPENDENCIES
    DIRECTORIES "${CMAKE_BINARY_DIR}/root-all\\bin\\lib1"
    PRE_INCLUDE_REGEXES "^(lib)?test\\.dll$"
    PRE_EXCLUDE_REGEXES ".*"
    POST_INCLUDE_REGEXES "\\\\lib1/(lib)?test\\.dll$"
    POST_EXCLUDE_REGEXES ".*"
)

install(
  TARGETS exe
  DESTINATION results_new
  RUNTIME_DEPENDENCIES
    DIRECTORIES "${CMAKE_BINARY_DIR}/root-all\\bin\\lib1"
    PRE_INCLUDE_REGEXES "^(lib)?test\\.dll$"
    PRE_EXCLUDE_REGEXES ".*"
    POST_INCLUDE_REGEXES "/lib1/(lib)?test\\.dll$"
    POST_EXCLUDE_REGEXES ".*"
)

install(
  TARGETS exe
  DESTINATION results_any
  RUNTIME_DEPENDENCIES
    DIRECTORIES "${CMAKE_BINARY_DIR}/root-all\\bin\\lib1"
    PRE_INCLUDE_REGEXES "^(lib)?test\\.dll$"
    PRE_EXCLUDE_REGEXES ".*"
    POST_INCLUDE_REGEXES "(\\\\|/)lib1/(lib)?test\\.dll$"
    POST_EXCLUDE_REGEXES ".*"
)

install(
  TARGETS exe
  DESTINATION results_any_forward
  RUNTIME_DEPENDENCIES
    DIRECTORIES "${CMAKE_BINARY_DIR}/root-all/bin/lib1"
    PRE_INCLUDE_REGEXES "^(lib)?test\\.dll$"
    PRE_EXCLUDE_REGEXES ".*"
    POST_INCLUDE_REGEXES "(\\\\|/)lib1/(lib)?test\\.dll$"
    POST_EXCLUDE_REGEXES ".*"
)

install(
  CODE [[
    function(check_installed_lib directory out_var)
      file(GLOB_RECURSE actual
        LIST_DIRECTORIES FALSE
        RELATIVE ${CMAKE_INSTALL_PREFIX}/${directory}
        ${CMAKE_INSTALL_PREFIX}/${directory}/*.dll
      )

      if(actual)
        list(SORT actual)
      endif()

      set(${out_var} "${actual}" PARENT_SCOPE)
    endfunction()

    check_installed_lib("results_old" results_old)
    check_installed_lib("results_new" results_new)
    check_installed_lib("results_any" results_any)
    check_installed_lib("results_any_forward" results_any_forward)

    if(NOT results_any)
      message(SEND_ERROR "Any dependencies are empty: `${results_any}`")
    endif()

    if(NOT results_any_forward)
      message(SEND_ERROR "Any forward dependencies are empty: `${results_any_forward}`")
    endif()
  ]]
)
