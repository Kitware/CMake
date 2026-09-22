set(check "${CMAKE_CURRENT_LIST_DIR}/check-special-args.cmake")
set(CMAKE_C_COMPILER_LAUNCHER
  "${CMAKE_COMMAND};-P;${check};--;compile;;semi\\;colon;with space")
include(C-common.cmake)
