PROJECT(StringFileTest)
INCLUDE_DIRECTORIES(${StringFileTest_BINARY_DIR})

# Read file test
FILE(READ "${CMAKE_CURRENT_SOURCE_DIR}/InputFile.h.in" infile)

# String test
STRING(REGEX MATCH "[cC][mM][aA][kK][eE]" rmvar "CMake is great")
STRING(REGEX MATCHALL "[cC][mM][aA][kK][eE]" rmallvar "CMake is better than cmake or CMake")
STRING(REGEX REPLACE "[Aa][uU][tT][oO]([cC][oO][nN][fF]|[mM][aA][kK][eE])"
  "CMake" rrepvar "People should use Autoconf and Automake")
STRING(COMPARE EQUAL "CMake" "Autoconf" nceqvar)
STRING(COMPARE EQUAL "CMake" "CMake" ceqvar)
STRING(COMPARE NOTEQUAL "CMake" "Autoconf" cneqvar)
STRING(COMPARE NOTEQUAL "CMake" "CMake" ncneqvar)
STRING(COMPARE LESS "before" "after" nclvar)
STRING(COMPARE LESS "max" "min" clvar)
STRING(COMPARE GREATER "before" "after" cgvar)
STRING(COMPARE GREATER "max" "min" ncgvar)
STRING(ASCII 67 109 97 107 101 savar)
STRING(TOUPPER "CMake" tuvar)
STRING(TOLOWER "CMake" tlvar)

# Escaping test
SET(var "\\ \" \  \t \n \r \# \( \) \0")
MESSAGE("Output: [${var}]")
SET(var \\ \" \  \t \n \r \# \( \) \0)
MESSAGE("Output: [${var}]")

# Make directories test
FILE(MAKE_DIRECTORY 
  "${CMAKE_CURRENT_BINARY_DIR}/Includes" 
  "${CMAKE_CURRENT_BINARY_DIR}/Directory1"
  "${CMAKE_CURRENT_BINARY_DIR}/Directory2"
  )

# Write results to the file (test write file)
SET(file "${CMAKE_CURRENT_BINARY_DIR}/Includes/Values.h")
FILE(WRITE "${file}" "/* this file is generated */\n")
FOREACH(var
    rmvar
    rmallvar
    rrepvar
    nceqvar
    ceqvar
    cneqvar
    ncneqvar
    nclvar
    clvar
    cgvar
    ncgvar
    savar
    tuvar
    tlvar)
  FILE(APPEND "${file}" "#define ${var} \"${${var}}\"\n")
ENDFOREACH(var)

# Test configuration of the string
SET(TEST_DEFINED 123)
SET(TEST_NOT_DEFINED)
STRING(CONFIGURE "${infile}" infile @ONLY)

# Write include file to a file
STRING(REGEX REPLACE "includefile" "${file}" outfile "${infile}")
FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/OutputFile.h" "${outfile}")

# Test file glob
FILE(GLOB_RECURSE src_files "${CMAKE_CURRENT_SOURCE_DIR}/*")
MESSAGE("Files in ${CMAKE_CURRENT_SOURCE_DIR} are ${src_files}")
SET(expr "${CMAKE_CURRENT_SOURCE_DIR}/[sS][!a-su-zA-Z0-9][^a-qs-zA-Z0-9]ing?ile*.cxx")
MESSAGE("Glob expression is [${expr}].")
FILE(GLOB src_files "${expr}")
MESSAGE("Globbed files [${src_files}].")

ADD_EXECUTABLE(StringFileTest ${src_files})

# Test FOREACH range
MESSAGE("Cheack if FOREACH with RANGE works")
MACRO(TEST_RANGE ARGS CHECK)
  SET(r)
  FOREACH(a RANGE ${ARGS})
    SET(r ${r} ${a})
  ENDFOREACH(a)
  MESSAGE("FOREACH with RANGE ${ARGS} produces ${r}")
  IF("x${r}x" MATCHES "^x${CHECK}x$")
  ELSE("x${r}x" MATCHES "^x${CHECK}x$")
    MESSAGE(SEND_ERROR "The range resulted in: ${r} should be ${CHECK}")
  ENDIF("x${r}x" MATCHES "^x${CHECK}x$")
ENDMACRO(TEST_RANGE)
TEST_RANGE("5" "0;1;2;3;4;5")
TEST_RANGE("3;5" "3;4;5")
TEST_RANGE("5;3" "5;4;3")
TEST_RANGE("3;10;2" "3;5;7;9")
TEST_RANGE("10;0;-3" "10;7;4;1")
