include(${CMAKE_CURRENT_LIST_DIR}/check-common.cmake)

string(REPLACE ${path_prefix} "" test_shell_path ${test_shell_path})

if(MSYS)
  check(test_shell_path [[/c/shell/path]])
elseif(WIN32)
  check(test_shell_path [[c:\shell\path]])
else()
  check(test_shell_path [[/shell/path]])
endif()
