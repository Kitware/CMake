project(Preprocess)

# This test is meant both as a test and as a reference for supported
# syntax on native tool command lines.

# We need ansi C support.
IF(CMAKE_ANSI_CFLAGS)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_ANSI_CFLAGS}")
ENDIF(CMAKE_ANSI_CFLAGS)

#-----------------------------------------------------------------------------
# Construct a C-string literal to test passing through a definition on
# the command line.  We configure the value into a header so it can be
# checked in the executable at runtime.  The semicolon is handled
# specially because it needs to be escaped in the COMPILE_DEFINITIONS
# property value to avoid separating definitions but the string value
# must not have it escaped inside the configured header.
set(STRING_EXTRA "")

if("${CMAKE_GENERATOR}" MATCHES "Make" AND MSVC)
  set(NMAKE 1)
endif("${CMAKE_GENERATOR}" MATCHES "Make" AND MSVC)

if(NOT BORLAND AND NOT "${CMAKE_GENERATOR}" MATCHES "Visual Studio 7$")
  # Borland, VS70 IDE: ;
  # The Borland compiler will simply not accept a non-escaped semicolon
  # on the command line.  If it is escaped \; then the escape character
  # shows up in the preprocessing output too.
  #
  # The VS 7.0 IDE separates definitions on semicolons and commas with
  # no regard for quotes.  Fortunately VS 7.1 and above are okay.
  set(SEMICOLON "\;")
endif(NOT BORLAND AND NOT "${CMAKE_GENERATOR}" MATCHES "Visual Studio 7$")

if(NOT BORLAND AND NOT WATCOM)
  # Borland, WMake: multiple spaces
  # The make tool seems to remove extra whitespace from inside
  # quoted strings when passing to the compiler.  It does not have
  # trouble passing to other tools, and the compiler may be directly
  # invoked from the command line.
  set(STRING_EXTRA "${STRING_EXTRA}  ")
endif(NOT BORLAND AND NOT WATCOM)

if(NOT "${CMAKE_GENERATOR}" MATCHES "Visual Studio")
  # VS: ,
  # Visual Studio will not accept a comma in the value of a definition.
  # The comma-separated list of PreprocessorDefinitions in the project
  # file seems to be parsed before the content of entries is examined.
  set(STRING_EXTRA "${STRING_EXTRA},")
endif(NOT "${CMAKE_GENERATOR}" MATCHES "Visual Studio")

if(NOT MINGW)
  # MinGW: &
  # When inside -D"FOO=\"a & b\"" MinGW make wants -D"FOO=\"a "&" b\""
  # but it does not like quoted ampersand elsewhere.
  set(STRING_EXTRA "${STRING_EXTRA}&")
endif(NOT MINGW)

if(NOT MINGW)
  # MinGW: |
  # When inside -D"FOO=\"a | b\"" MinGW make wants -D"FOO=\"a "|" b\""
  # but it does not like quoted pipe elsewhere.
  set(STRING_EXTRA "${STRING_EXTRA}|")
endif(NOT MINGW)

if(NOT BORLAND AND NOT MINGW AND NOT NMAKE)
  # Borland, NMake, MinGW: ^
  # When inside -D"FOO=\"a ^ b\"" they make wants -D"FOO=\"a "^" b\""
  # but do not like quoted carrot elsewhere.  In NMake the non-quoted
  # syntax works when the flags are not in a make variable.
  set(STRING_EXTRA "${STRING_EXTRA}^")
endif(NOT BORLAND AND NOT MINGW AND NOT NMAKE)

if(NOT BORLAND AND NOT MINGW AND NOT NMAKE)
  # Borland, MinGW: < >
  # Angle-brackets have funny behavior that is hard to escape.
  set(STRING_EXTRA "${STRING_EXTRA}<>")
endif(NOT BORLAND AND NOT MINGW AND NOT NMAKE)

# General: \"
# Make tools do not reliably accept \\\" syntax:
#  - MinGW and MSYS make tools crash with \\\"
#  - Borland make actually wants a mis-matched quote \\"
#    or $(BACKSLASH)\" where BACKSLASH is a variable set to \\
#  - VS IDE gets confused about the bounds of the definition value \\\"
#  - NMake is okay with just \\\"
if(NMAKE OR "${CMAKE_GENERATOR}" MATCHES "Unix Makefiles")
  set(STRING_EXTRA "${STRING_EXTRA}\\\"")
endif(NMAKE OR "${CMAKE_GENERATOR}" MATCHES "Unix Makefiles")

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
set(STRING_VALUE "hello `~!@$%*)(_+-=}{][:'.?/ ${STRING_EXTRA}world")

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

set(EXPR "x*y+!(x==(y+1*2))*f(x%2)")

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
if(XCODE)
  set(PREPROCESS_XCODE 1)
endif(XCODE)
if("${CMAKE_GENERATOR}" MATCHES "Visual Studio 6")
  set(PREPROCESS_VS6 1)
  set(VS6 _vs6)
endif("${CMAKE_GENERATOR}" MATCHES "Visual Studio 6")

# Test old-style definitions.
add_definitions(-DOLD_DEF -DOLD_EXPR=2)

set(FILE_PATH "${Preprocess_SOURCE_DIR}/file_def.h")
set(TARGET_PATH "${Preprocess_SOURCE_DIR}/target_def.h")

# Create a list of definition property strings.
set(TARGET_DEFS "TARGET_DEF")
set(FILE_DEFS "FILE_DEF")

# Add definitions with values.  VS6 does not support this.
if(NOT PREPROCESS_VS6)
  list(APPEND TARGET_DEFS
    "TARGET_STRING=\"${STRING_VALUE}${SEMICOLON}\""
    "TARGET_EXPR=${EXPR}"
    "TARGET_PATH=\"${TARGET_PATH}\""
    )
  list(APPEND FILE_DEFS
    "FILE_STRING=\"${STRING_VALUE}${SEMICOLON}\""
    "FILE_EXPR=${EXPR}"
    "FILE_PATH=\"${FILE_PATH}\""
    )
endif(NOT PREPROCESS_VS6)

add_executable(Preprocess preprocess.c preprocess${VS6}.cxx)
set_target_properties(Preprocess PROPERTIES
  COMPILE_DEFINITIONS "${TARGET_DEFS}"
  COMPILE_DEFINITIONS_DEBUG "TARGET_DEF_DEBUG"
  COMPILE_DEFINITIONS_RELEASE "TARGET_DEF_RELEASE"
  )
set_source_files_properties(preprocess.c preprocess${VS6}.cxx PROPERTIES
  COMPILE_DEFINITIONS "${FILE_DEFS}"
  COMPILE_DEFINITIONS_DEBUG "FILE_DEF_DEBUG"
  COMPILE_DEFINITIONS_RELEASE "FILE_DEF_RELEASE"
  )

# Helper target for running test manually in build tree.
add_custom_target(drive COMMAND Preprocess)

# Configure the header file with the desired string value.
if(SEMICOLON)
  set(STRING_VALUE "${STRING_VALUE};")
endif(SEMICOLON)
configure_file(${Preprocess_SOURCE_DIR}/preprocess.h.in
               ${Preprocess_BINARY_DIR}/preprocess.h)
include_directories(${Preprocess_BINARY_DIR})
