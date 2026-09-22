set(check "${CMAKE_CURRENT_LIST_DIR}/check-special-args.cmake")
set(CMAKE_C_LINKER_LAUNCHER
  "${CMAKE_COMMAND};-P;${check};--;link;;semi\\;colon;with space")
include(C-common.cmake)
