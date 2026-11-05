include(${CMAKE_CURRENT_LIST_DIR}/check.cmake)
find_package(FLEX REQUIRED)

flex_target(Options scanner.l scanner1.c OPTIONS --warn --noline)
check_var(FLEX_Options_OPTIONS "--warn;--noline")
check_var(FLEX_Options_COMPILE_FLAGS "--warn;--noline")

flex_target(CompileFlags scanner.l scanner2.c COMPILE_FLAGS "--warn --noline")
check_var(FLEX_CompileFlags_OPTIONS "--warn;--noline")

flex_target(Both scanner.l scanner3.c COMPILE_FLAGS "--warn" OPTIONS --noline)
check_var(FLEX_Both_OPTIONS "--warn;--noline")

flex_target(Genex scanner.l scanner4.c OPTIONS $<$<CONFIG:Debug>:--debug>)
check_var(FLEX_Genex_OPTIONS "$<$<CONFIG:Debug>:--debug>")

flex_target(WithDefines scanner.l scanner5.c OPTIONS --warn DEFINES_FILE s5.h)
check_var(FLEX_WithDefines_OPTIONS
  "--warn;--header-file=${CMAKE_CURRENT_BINARY_DIR}/s5.h")
