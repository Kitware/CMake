include(${CMAKE_CURRENT_LIST_DIR}/check.cmake)
find_package(BISON REQUIRED)

bison_target(Options parser.y parser1.c OPTIONS -Wall --no-lines)
check_var(BISON_Options_OPTIONS "-Wall;--no-lines;-d")
check_var(BISON_Options_COMPILE_FLAGS "-Wall;--no-lines;-d")

bison_target(CompileFlags parser.y parser2.c COMPILE_FLAGS "-Wall --no-lines")
check_var(BISON_CompileFlags_OPTIONS "-Wall;--no-lines;-d")

bison_target(Both parser.y parser3.c COMPILE_FLAGS "-Wall" OPTIONS --no-lines)
check_var(BISON_Both_OPTIONS "-Wall;--no-lines;-d")

bison_target(Genex parser.y parser4.c OPTIONS $<$<CONFIG:Debug>:-t>)
check_var(BISON_Genex_OPTIONS "$<$<CONFIG:Debug>:-t>;-d")

bison_target(WithDefines parser.y parser5.c OPTIONS -Wall DEFINES_FILE p5.h)
check_var(BISON_WithDefines_OPTIONS "-Wall;--defines=p5.h")
