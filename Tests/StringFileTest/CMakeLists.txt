cmake_minimum_required (VERSION 2.6)
PROJECT(StringFileTest)
INCLUDE_DIRECTORIES(${StringFileTest_BINARY_DIR})

# Read file test
FILE(READ "${CMAKE_CURRENT_SOURCE_DIR}/InputFile.h.in" infile)

# Test reading a binary file into hex representation
FILE(READ "${CMAKE_CURRENT_SOURCE_DIR}/test.bin" hexContents HEX)

IF("${hexContents}" STREQUAL "0001027700")
  MESSAGE("FILE(READ HEX) correctly read [${hexContents}]")
ELSE("${hexContents}" STREQUAL "0001027700")
  MESSAGE(SEND_ERROR "FILE(READ HEX) incorrectly read [${hexContents}], but expected was [0001027700]")
ENDIF("${hexContents}" STREQUAL "0001027700")

# FILE(STRINGS) test
FILE(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/InputFile.h.in" infile_strings
  LIMIT_COUNT 1 LIMIT_INPUT 1024 LIMIT_OUTPUT 1024
  LENGTH_MINIMUM 10 LENGTH_MAXIMUM 23 REGEX include NEWLINE_CONSUME)
SET(infile_strings_goal "#include \"includefile\"\n")
IF("${infile_strings}" STREQUAL "${infile_strings_goal}")
  MESSAGE("FILE(STRINGS) correctly read [${infile_strings}]")
ELSE("${infile_strings}" STREQUAL "${infile_strings_goal}")
  MESSAGE(SEND_ERROR
    "FILE(STRINGS) incorrectly read [${infile_strings}]")
ENDIF("${infile_strings}" STREQUAL "${infile_strings_goal}")

# test reading a file and getting its binary data as hex string
FILE(READ "${CMAKE_CURRENT_SOURCE_DIR}/main.srec" infilehex LIMIT 4 HEX)
IF(NOT "${infilehex}" STREQUAL "53313036")
  MESSAGE(SEND_ERROR
    "FILE(READ ... HEX) error, read: \"${infilehex}\", expected \"53313036\"")
ENDIF(NOT "${infilehex}" STREQUAL "53313036")


# test that FILE(STRINGS) also work with Intel hex and Motorola S-record files
# this file has been created with "sdcc main.c"
FILE(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/main.ihx" infile_strings REGEX INFO)
SET(infile_strings_goal "INFO:compiler\\[SDCC-HEX\\]")
IF("${infile_strings}" MATCHES "${infile_strings_goal}")
  MESSAGE("FILE(STRINGS) correctly read from hex file [${infile_strings}]")
ELSE("${infile_strings}" MATCHES "${infile_strings_goal}")
  MESSAGE(SEND_ERROR
    "FILE(STRINGS) incorrectly read from hex file [${infile_strings}]")
ENDIF("${infile_strings}" MATCHES "${infile_strings_goal}")

# this file has been created with "sdcc main.c --out-fmt-s19"
FILE(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/main.srec" infile_strings REGEX INFO)
SET(infile_strings_goal "INFO:compiler\\[SDCC-SREC\\]")
IF("${infile_strings}" MATCHES "${infile_strings_goal}")
  MESSAGE("FILE(STRINGS) correctly read from srec file [${infile_strings}]")
ELSE("${infile_strings}" MATCHES "${infile_strings_goal}")
  MESSAGE(SEND_ERROR
    "FILE(STRINGS) incorrectly read from srec file [${infile_strings}]")
ENDIF("${infile_strings}" MATCHES "${infile_strings_goal}")

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
STRING(REPLACE "Autoconf" "CMake" repvar "People should use Autoconf")

IF("abc" STREQUAL "xyz")
  MESSAGE(SEND_ERROR "Problem with the IF(STREQUAL), \"abc\" and \"xyz\" considered equal")
ENDIF("abc" STREQUAL "xyz")

IF("CMake is cool" MATCHES "(CMake) (is).+")
  IF(NOT "${CMAKE_MATCH_0}" STREQUAL "CMake is cool")
    MESSAGE(SEND_ERROR "CMAKE_MATCH_0 wrong: \"${CMAKE_MATCH_0}\", expected \"CMake is cool\"")
  ENDIF(NOT "${CMAKE_MATCH_0}" STREQUAL "CMake is cool")
  IF(NOT "${CMAKE_MATCH_1}" STREQUAL "CMake")
    MESSAGE(SEND_ERROR "CMAKE_MATCH_1 wrong: \"${CMAKE_MATCH_1}\", expected \"CMake\"")
  ENDIF(NOT "${CMAKE_MATCH_1}" STREQUAL "CMake")
  IF(NOT "${CMAKE_MATCH_2}" STREQUAL "is")
    MESSAGE(SEND_ERROR "CMAKE_MATCH_2 wrong: \"${CMAKE_MATCH_2}\", expected \"is\"")
  ENDIF(NOT "${CMAKE_MATCH_2}" STREQUAL "is")
ELSE("CMake is cool" MATCHES "(CMake) (is).+")
  MESSAGE(SEND_ERROR "Problem with the IF(MATCHES), no match found")
ENDIF("CMake is cool" MATCHES "(CMake) (is).+")

STRING(REGEX MATCH "(People).+CMake" matchResultVar "People should use CMake")
IF(NOT "${matchResultVar}" STREQUAL "People should use CMake")
  MESSAGE(SEND_ERROR "STRING(REGEX MATCH) problem: \"${matchResultVar}\", expected \"People should use CMake\"")
ENDIF(NOT "${matchResultVar}" STREQUAL "People should use CMake")
IF(NOT "${CMAKE_MATCH_0}" STREQUAL "People should use CMake")
  MESSAGE(SEND_ERROR "CMAKE_MATCH_0 wrong: \"${CMAKE_MATCH_0}\", expected \"People should use CMake\"")
ENDIF(NOT "${CMAKE_MATCH_0}" STREQUAL "People should use CMake")
IF(NOT "${CMAKE_MATCH_1}" STREQUAL "People")
  MESSAGE(SEND_ERROR "CMAKE_MATCH_1 wrong: \"${CMAKE_MATCH_1}\", expected \"People\"")
ENDIF(NOT "${CMAKE_MATCH_1}" STREQUAL "People")
IF(NOT "${CMAKE_MATCH_2}" STREQUAL "")
  MESSAGE(SEND_ERROR "CMAKE_MATCH_2 wrong: \"${CMAKE_MATCH_2}\", expected empty string")
ENDIF(NOT "${CMAKE_MATCH_2}" STREQUAL "")


STRING(STRIP "
  ST1
  " ST1)
STRING(STRIP "ST2 " ST2)
STRING(STRIP "  ST3" ST3)

FOREACH(var ST1 ST2 ST3)
  IF("${var}" STREQUAL "${${var}}")
    MESSAGE("[${var}] == [${${var}}]")
  ELSE("${var}" STREQUAL "${${var}}")
    MESSAGE(SEND_ERROR "Problem with the STRIP command for ${var}: [${${var}}]")
  ENDIF("${var}" STREQUAL "${${var}}")
ENDFOREACH(var)

STRING(SUBSTRING "People should use Autoconf" 7 10 substringres)
SET(substringres "Everybody ${substringres} CMake")

STRING(LENGTH ${substringres} lengthres)

FILE(RELATIVE_PATH relpath "/usr/local/bin" "/usr/X11R6/bin/xnest")

# Escaping test
SET(var "\\ \" \  \t \n \r \# \( \) \0")
MESSAGE("Output: [${var}]")
SET(var \\ \" \  \t \n \r \# \( \) \0)
MESSAGE("Output: [${var}]")

# Make-style unquoted argument test
SET(var $(VAR1)$(VAR2)/$(VAR3))
MESSAGE("Output: [${var}]")
STRING(COMPARE EQUAL "${var}" "$(VAR1)$(VAR2)/$(VAR3)" result)
IF(NOT result)
  MESSAGE(SEND_ERROR "Unquoted $(VAR) syntax is broken.")
ENDIF(NOT result)

# Obscure environment variable name
SET("ENV{x+(y)}" "Obscure environment variable value")
MESSAGE("Output: [$ENV{x+(y)}]")
IF(NOT "$ENV{x+(y)}" STREQUAL "Obscure environment variable value")
  MESSAGE(SEND_ERROR "Environment variable \"ENV{x+(y)}\" does not work.")
ENDIF()

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
    repvar
    relpath
    substringres
    lengthres
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

# Verify that the file was created recently.
IF(NOT "${file}" IS_NEWER_THAN "${CMAKE_CURRENT_SOURCE_DIR}/InputFile.h.in")
  MESSAGE(FATAL_ERROR "IF(FILE_IS_NEWER) does not seem to work.")
ENDIF(NOT "${file}" IS_NEWER_THAN "${CMAKE_CURRENT_SOURCE_DIR}/InputFile.h.in")

# Test configuration of the string
SET(TEST_DEFINED 123)
SET(TEST_NOT_DEFINED)
STRING(CONFIGURE "${infile}" infile+-/out @ONLY)
SET(infile "${infile+-/out}")

# Write include file to a file
STRING(REGEX REPLACE "includefile" "${file}" outfile "${infile}")
FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/OutputFile.h-tmp" "${outfile}")
FILE(RENAME "${CMAKE_CURRENT_BINARY_DIR}/OutputFile.h-tmp"
            "${CMAKE_CURRENT_BINARY_DIR}/OutputFile.h")

# Test file copy with relative paths
FILE(COPY .
  DESTINATION src
  FILE_PERMISSIONS OWNER_READ # test no OWNER_WRITE
  DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
  FILES_MATCHING PATTERN *.cxx # Only copy the main source file
  REGEX /src$ EXCLUDE # Block recursion for in-source build
  )

# Test file glob
FILE(GLOB_RECURSE src_files "${CMAKE_CURRENT_SOURCE_DIR}/*")
MESSAGE("Files in ${CMAKE_CURRENT_SOURCE_DIR} are ${src_files}")
SET(expr "${CMAKE_CURRENT_BINARY_DIR}/src/[sS][!a-su-zA-Z0-9][^a-qs-zA-Z0-9]ing?ile*.cxx")
MESSAGE("Glob expression is [${expr}].")
FILE(GLOB src_files "${expr}")
MESSAGE("Globbed files [${src_files}].")
ADD_EXECUTABLE(StringFileTest ${src_files})

SET(expr "${CMAKE_CURRENT_SOURCE_DIR}/../*.cxx")
FILE(GLOB_RECURSE rel_src_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/.." "${expr}")
MESSAGE("Globbed files [${rel_src_files}].")

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

# Test FOREACH IN signature
set(list1 "" a "")
set(list2 a "" b)
set(var_)
set(var_a)
set(var_b)
foreach(item IN LISTS list1 list2 ITEMS "" a "")
  set(var_${item} "${var_${item}}x")
endforeach(item)
if(NOT "${var_}" STREQUAL "xxxxx")
  message(FATAL_ERROR "count incorrect for \"\": [${var_}]")
endif()
if(NOT "${var_a}" STREQUAL "xxx")
  message(FATAL_ERROR "count incorrect for \"a\": [${var_a}]")
endif()
if(NOT "${var_b}" STREQUAL "x")
  message(FATAL_ERROR "count incorrect \"b\": [${var_b}]")
endif()

# Test SUBSTRING command
SET(ST_INPUTSTRING "0123456789")
STRING(SUBSTRING ${ST_INPUTSTRING} 3 0 ST_EMPTY)
STRING(SUBSTRING ${ST_INPUTSTRING} 1 1 ST_ONE)
STRING(SUBSTRING ${ST_INPUTSTRING} 0 10 ST_ALL)
STRING(SUBSTRING ${ST_INPUTSTRING} 0 -1 ST_ALL_MINUS)
STRING(SUBSTRING ${ST_INPUTSTRING} 9 -1 ST_NINE)

IF(ST_EMPTY)
  MESSAGE(SEND_ERROR "SUBSTRING with length 0 does not return an empty string")
ENDIF(ST_EMPTY)
IF(NOT ST_ONE STREQUAL "1")
  MESSAGE(SEND_ERROR "SUBSTING command does not cut the correct selected character, was \"" ${ST_ONE} "\", should be \"1\"")
ENDIF(NOT ST_ONE STREQUAL "1")
IF(NOT ST_INPUTSTRING STREQUAL ST_ALL)
  MESSAGE(SEND_ERROR "SUBSTRING does not return the whole string when selected with length")
ENDIF(NOT ST_INPUTSTRING STREQUAL ST_ALL)
IF(NOT ST_INPUTSTRING STREQUAL ST_ALL_MINUS)
  MESSAGE(SEND_ERROR "SUBSTRING does not return the whole string when selected with -1")
ENDIF(NOT ST_INPUTSTRING STREQUAL ST_ALL_MINUS)
IF(NOT ST_NINE STREQUAL "9")
  MESSAGE(SEND_ERROR "SUBSTRING does not return the tail when selected with -1")
ENDIF(NOT ST_NINE STREQUAL "9")
