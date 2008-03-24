cmake_minimum_required(VERSION 2.6)
project(Preprocess)

# This test is meant both as a test and as a reference for supported
# syntax on native tool command lines.

# We need ansi C support.
IF(CMAKE_ANSI_CFLAGS)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_ANSI_CFLAGS}")
ENDIF(CMAKE_ANSI_CFLAGS)

# Determine the build tool being used.  Not all characters can be
# escaped for all build tools.  This test checks all characters known
# to work with each tool and documents those known to not work.
if("${CMAKE_GENERATOR}" MATCHES "Xcode")
  set(PP_XCODE 1)
endif("${CMAKE_GENERATOR}" MATCHES "Xcode")
if("${CMAKE_GENERATOR}" MATCHES "Visual Studio 6")
  set(PP_VS6 1)
endif("${CMAKE_GENERATOR}" MATCHES "Visual Studio 6")
if("${CMAKE_GENERATOR}" MATCHES "Unix Makefiles")
  set(PP_UMAKE 1)
endif("${CMAKE_GENERATOR}" MATCHES "Unix Makefiles")
if("${CMAKE_GENERATOR}" MATCHES "NMake Makefiles")
  set(PP_NMAKE 1)
endif("${CMAKE_GENERATOR}" MATCHES "NMake Makefiles")
if("${CMAKE_GENERATOR}" MATCHES "MinGW Makefiles")
  set(PP_MINGW 1)
endif("${CMAKE_GENERATOR}" MATCHES "MinGW Makefiles")
if("${CMAKE_GENERATOR}" MATCHES "Borland Makefiles")
  set(PP_BORLAND 1)
endif("${CMAKE_GENERATOR}" MATCHES "Borland Makefiles")
if("${CMAKE_GENERATOR}" MATCHES "Watcom WMake")
  set(PP_WATCOM 1)
endif("${CMAKE_GENERATOR}" MATCHES "Watcom WMake")
if("${CMAKE_GENERATOR}" MATCHES "Visual Studio 7$")
  set(PP_VS70 1)
endif("${CMAKE_GENERATOR}" MATCHES "Visual Studio 7$")
if("${CMAKE_GENERATOR}" MATCHES "Visual Studio")
  set(PP_VS 1)
endif("${CMAKE_GENERATOR}" MATCHES "Visual Studio")

# Some tests below check the PP_* variables set above.  They are meant
# to test the case that the build tool is at fault.  Other tests below
# check the compiler that will be used when the compiler is at fault
# (does not work even from a command shell).

#-----------------------------------------------------------------------------
# Construct a C-string literal to test passing through a definition on
# the command line.  We configure the value into a header so it can be
# checked in the executable at runtime.  The semicolon is handled
# specially because it needs to be escaped in the COMPILE_DEFINITIONS
# property value to avoid separating definitions but the string value
# must not have it escaped inside the configured header.
set(STRING_EXTRA "")

if(NOT BORLAND AND NOT PP_VS70)
  # Borland, VS70 IDE: ;
  # The Borland compiler will simply not accept a non-escaped semicolon
  # on the command line.  If it is escaped \; then the escape character
  # shows up in the preprocessing output too.
  #
  # The VS 7.0 IDE separates definitions on semicolons and commas with
  # no regard for quotes.  Fortunately VS 7.1 and above are okay.
  set(SEMICOLON "\;")
endif(NOT BORLAND AND NOT PP_VS70)

if(NOT PP_BORLAND AND NOT PP_WATCOM)
  # Borland, WMake: multiple spaces
  # The make tool seems to remove extra whitespace from inside
  # quoted strings when passing to the compiler.  It does not have
  # trouble passing to other tools, and the compiler may be directly
  # invoked from the command line.
  set(STRING_EXTRA "${STRING_EXTRA}  ")
endif(NOT PP_BORLAND AND NOT PP_WATCOM)

if(NOT PP_VS)
  # VS: ,
  # Visual Studio will not accept a comma in the value of a definition.
  # The comma-separated list of PreprocessorDefinitions in the project
  # file seems to be parsed before the content of entries is examined.
  set(STRING_EXTRA "${STRING_EXTRA},")
endif(NOT PP_VS)

if(NOT PP_MINGW)
  # MinGW: &
  # When inside -D"FOO=\"a & b\"" MinGW make wants -D"FOO=\"a "&" b\""
  # but it does not like quoted ampersand elsewhere.
  set(STRING_EXTRA "${STRING_EXTRA}&")
endif(NOT PP_MINGW)

if(NOT PP_MINGW)
  # MinGW: |
  # When inside -D"FOO=\"a | b\"" MinGW make wants -D"FOO=\"a "|" b\""
  # but it does not like quoted pipe elsewhere.
  set(STRING_EXTRA "${STRING_EXTRA}|")
endif(NOT PP_MINGW)

if(NOT PP_BORLAND AND NOT PP_MINGW AND NOT PP_NMAKE)
  # Borland, NMake, MinGW: ^
  # When inside -D"FOO=\"a ^ b\"" the make tools want -D"FOO=\"a "^" b\""
  # but do not like quoted carrot elsewhere.  In NMake the non-quoted
  # syntax works when the flags are not in a make variable.
  set(STRING_EXTRA "${STRING_EXTRA}^")
endif(NOT PP_BORLAND AND NOT PP_MINGW AND NOT PP_NMAKE)

if(NOT PP_BORLAND AND NOT PP_MINGW AND NOT PP_NMAKE)
  # Borland, MinGW: < >
  # Angle-brackets have funny behavior that is hard to escape.
  set(STRING_EXTRA "${STRING_EXTRA}<>")
endif(NOT PP_BORLAND AND NOT PP_MINGW AND NOT PP_NMAKE)

set(EXPR_OP1 "/")
if(NOT MSVC OR PP_NMAKE)
  # MSVC cl: %
  # When the cl compiler is invoked from the command line then % must
  # be written %% (to distinguish from %ENV% syntax).  However cl does
  # not seem to accept the syntax when it is invoked from inside a
  # make tool (nmake, mingw32-make, etc.).  Instead the argument must
  # be placed inside a response file.  Then cl accepts it because it
  # parses the response file as it would the normal windows command
  # line.  Currently only NMake supports running cl with a response
  # file.  Supporting other make tools would require CMake to generate
  # response files explicitly for each object file.
  set(STRING_EXTRA "${STRING_EXTRA}%")
  set(EXPR_OP1 "%")
endif(NOT MSVC OR PP_NMAKE)

# General: \"
# Make tools do not reliably accept \\\" syntax:
#  - MinGW and MSYS make tools crash with \\\"
#  - Borland make actually wants a mis-matched quote \\"
#    or $(BACKSLASH)\" where BACKSLASH is a variable set to \\
#  - VS IDE gets confused about the bounds of the definition value \\\"
#  - NMake is okay with just \\\"
if(PP_NMAKE OR PP_UMAKE)
  set(STRING_EXTRA "${STRING_EXTRA}\\\"")
endif(PP_NMAKE OR PP_UMAKE)

# General: #
# MSVC will not accept a # in the value of a string definition on the
# command line.  The character seems to be simply replaced by an
# equals =.  According to "cl -help" definitions may be specified by
# -DMACRO#VALUE as well as -DMACRO=VALUE.  It must be implemented by a
# simple search-and-replace.
#
# The Borland compiler will parse both # and \# as just # but the make
# tool seems to want \# sometimes and not others.
#
# Unix make does not like # in variable settings without extra
# escaping.  This could probably be fixed but since MSVC does not
# support it and it is not an operator it is not worthwhile.

# Compose the final test string.
set(STRING_VALUE "hello `~!@$*)(_+-=}{][:'.?/ ${STRING_EXTRA}world")

#-----------------------------------------------------------------------------
# Function-style macro command-line support:
#   - Borland does not support
#   - MSVC does not support
#   - Watcom does not support
#   - GCC supports

# Too few platforms support this to bother implementing.
# People can just configure headers with the macros.

#-----------------------------------------------------------------------------
# Construct a sample expression to pass as a macro definition.

set(EXPR "x*y+!(x==(y+1*2))*f(x${EXPR_OP1}2)")

if(NOT WATCOM)
  # Watcom does not support - or / because it parses them as options.
  set(EXPR "${EXPR}+y/x-x")
endif(NOT WATCOM)

#-----------------------------------------------------------------------------

# Inform the test if the debug configuration is getting built.
# The NDEBUG definition takes care of this for release.
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DPREPROCESS_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DPREPROCESS_DEBUG")

# Inform the test if it built from Xcode or VS6 IDE.
if(PP_XCODE)
  set(PREPROCESS_XCODE 1)
endif(PP_XCODE)
if(PP_VS6)
  set(PREPROCESS_VS6 1)
  set(VS6 _vs6)
endif(PP_VS6)

# Test old-style definitions.
add_definitions(-DOLD_DEF -DOLD_EXPR=2)

# Make sure old-style definitions are converted to directory property.
if(PREPROCESS_VS6)
  set(OLD_DEFS_EXPECTED "OLD_DEF")
else(PREPROCESS_VS6)
  set(OLD_DEFS_EXPECTED "OLD_DEF;OLD_EXPR=2")
endif(PREPROCESS_VS6)
get_property(OLD_DEFS DIRECTORY PROPERTY COMPILE_DEFINITIONS)
if(NOT "${OLD_DEFS}" STREQUAL "${OLD_DEFS_EXPECTED}")
  message(SEND_ERROR "add_definitions not converted to directory property!")
endif(NOT "${OLD_DEFS}" STREQUAL "${OLD_DEFS_EXPECTED}")

add_executable(Preprocess preprocess.c preprocess${VS6}.cxx)

set(FILE_PATH "${Preprocess_SOURCE_DIR}/file_def.h")
set(TARGET_PATH "${Preprocess_SOURCE_DIR}/target_def.h")

# Set some definition properties.
foreach(c "" "_DEBUG" "_RELEASE")
  set_property(
    DIRECTORY .
    APPEND PROPERTY COMPILE_DEFINITIONS${c} "DIRECTORY_DEF${c}"
    )
  set_property(
    TARGET Preprocess
    PROPERTY COMPILE_DEFINITIONS${c} "TARGET_DEF${c}"
    )
  set_property(
    SOURCE preprocess.c preprocess${VS6}.cxx
    PROPERTY COMPILE_DEFINITIONS${c} "FILE_DEF${c}"
    )
endforeach(c)

# Add definitions with values.  VS6 does not support this.
if(NOT PREPROCESS_VS6)
  set_property(
    TARGET Preprocess
    APPEND PROPERTY COMPILE_DEFINITIONS
    "TARGET_STRING=\"${STRING_VALUE}${SEMICOLON}\""
    "TARGET_EXPR=${EXPR}"
    "TARGET_PATH=\"${TARGET_PATH}\""
    )
  set_property(
    SOURCE preprocess.c preprocess${VS6}.cxx
    APPEND PROPERTY COMPILE_DEFINITIONS
    "FILE_STRING=\"${STRING_VALUE}${SEMICOLON}\""
    "FILE_EXPR=${EXPR}"
    "FILE_PATH=\"${FILE_PATH}\""
    )
endif(NOT PREPROCESS_VS6)

# Helper target for running test manually in build tree.
add_custom_target(drive COMMAND Preprocess)

# Configure the header file with the desired string value.
if(SEMICOLON)
  set(STRING_VALUE "${STRING_VALUE};")
endif(SEMICOLON)
configure_file(${Preprocess_SOURCE_DIR}/preprocess.h.in
               ${Preprocess_BINARY_DIR}/preprocess.h)
include_directories(${Preprocess_BINARY_DIR})
