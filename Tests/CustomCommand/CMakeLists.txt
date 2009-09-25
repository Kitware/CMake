#
# Wrapping
#
cmake_minimum_required (VERSION 2.6)
PROJECT (CustomCommand)

ADD_SUBDIRECTORY(GeneratedHeader)

#
# Lib and exe path
#
SET (LIBRARY_OUTPUT_PATH 
  ${PROJECT_BINARY_DIR}/bin/ CACHE INTERNAL 
  "Single output directory for building all libraries.")

SET (EXECUTABLE_OUTPUT_PATH 
  ${PROJECT_BINARY_DIR}/bin/ CACHE INTERNAL 
  "Single output directory for building all executables.")

################################################################
#
#  First test using a compiled generator to create a .c file
#
################################################################
# add the executable that will generate the file
ADD_EXECUTABLE(generator generator.cxx)

GET_TARGET_PROPERTY(generator_PATH generator LOCATION)
MESSAGE("Location ${generator_PATH}")

################################################################
#
#  Test using a wrapper to wrap a header file
#
################################################################
# add the executable that will generate the file
ADD_EXECUTABLE(wrapper wrapper.cxx)

ADD_CUSTOM_COMMAND(
  OUTPUT ${PROJECT_BINARY_DIR}/wrapped.c ${PROJECT_BINARY_DIR}/wrapped_help.c
  DEPENDS wrapper
  MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/wrapped.h
  COMMAND ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/wrapper
  ${PROJECT_BINARY_DIR}/wrapped.c ${PROJECT_BINARY_DIR}/wrapped_help.c
  ${CMAKE_CFG_INTDIR} # this argument tests passing of the configuration
  VERBATIM # passing of configuration should work in this mode
  )

################################################################
#
#  Test creating files from a custom target
#
################################################################
ADD_CUSTOM_COMMAND(OUTPUT ${PROJECT_BINARY_DIR}/doc1.dvi
  DEPENDS ${PROJECT_SOURCE_DIR}/doc1.tex 
  COMMAND   ${CMAKE_COMMAND}  
  ARGS      -E copy ${PROJECT_SOURCE_DIR}/doc1.tex 
  ${PROJECT_BINARY_DIR}/doc1.dvi
  )

ADD_CUSTOM_COMMAND(OUTPUT ${PROJECT_BINARY_DIR}/doc1.h
  COMMAND ${CMAKE_COMMAND} -E echo " Copying doc1.dvi to doc1temp.h."
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/doc1.dvi
                                   ${PROJECT_BINARY_DIR}/doc1temp.h
  )
ADD_CUSTOM_COMMAND(OUTPUT ${PROJECT_BINARY_DIR}/doc1.h APPEND
  DEPENDS ${PROJECT_BINARY_DIR}/doc1.dvi
  COMMAND ${CMAKE_COMMAND} -E echo " Copying doc1temp.h to doc1.h."
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/doc1temp.h
                                   ${PROJECT_BINARY_DIR}/doc1.h
  COMMAND ${CMAKE_COMMAND} -E echo " Removing doc1temp.h."
  COMMAND ${CMAKE_COMMAND} -E remove -f ${PROJECT_BINARY_DIR}/doc1temp.h
  )

# Add custom command to generate foo.h.
ADD_CUSTOM_COMMAND(OUTPUT ${PROJECT_BINARY_DIR}/foo.h
  DEPENDS ${PROJECT_SOURCE_DIR}/foo.h.in
  COMMAND ${CMAKE_COMMAND} -E echo " Copying foo.h.in to foo.h."
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/foo.h.in
                                   ${PROJECT_BINARY_DIR}/foo.h
  )

# Add the location of foo.h to the include path.
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

# Test generation of a file to the build tree without full path.  As
# of CMake 2.6 custom command outputs specified by relative path go in
# the build tree.
ADD_CUSTOM_COMMAND(
  OUTPUT doc1.txt
  COMMAND ${CMAKE_COMMAND} -E echo "Example Document Target" > doc1.txt
  DEPENDS doc1.tex
  VERBATIM
  )

# Add a custom target to drive generation of doc1.h.
ADD_CUSTOM_TARGET(TDocument ALL
  COMMAND ${CMAKE_COMMAND} -E echo " Copying doc1.h to doc2.h."
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/doc1.h
                                   ${PROJECT_BINARY_DIR}/doc2.h
  DEPENDS ${PROJECT_BINARY_DIR}/doc1.h doc1.txt
  COMMENT "Running top-level TDocument commands"
  SOURCES doc1.tex
  )

# Setup a pre- and post-build pair that will fail if not run in the
# proper order.
ADD_CUSTOM_COMMAND(
  TARGET TDocument PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo " Writing doc1pre.txt."
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/doc1.tex ${PROJECT_BINARY_DIR}/doc1pre.txt
  COMMENT "Running TDocument pre-build commands"
  )
ADD_CUSTOM_COMMAND(
  TARGET TDocument POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo " Copying doc1pre.txt to doc2post.txt."
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/doc1pre.txt
                                   ${PROJECT_BINARY_DIR}/doc2post.txt
  COMMENT "Running TDocument post-build commands"
  )

################################################################
#
#  Test using a multistep generated file
#
################################################################
ADD_CUSTOM_COMMAND(OUTPUT ${PROJECT_BINARY_DIR}/foo.pre
  DEPENDS ${PROJECT_SOURCE_DIR}/foo.in
  COMMAND   ${CMAKE_COMMAND}  
  ARGS      -E copy ${PROJECT_SOURCE_DIR}/foo.in 
  ${PROJECT_BINARY_DIR}/foo.pre
  )

ADD_CUSTOM_COMMAND(OUTPUT ${PROJECT_BINARY_DIR}/foo.c
  DEPENDS   ${PROJECT_BINARY_DIR}/foo.pre 
  COMMAND   ${CMAKE_COMMAND}
  ARGS      -E copy ${PROJECT_BINARY_DIR}/foo.pre
  ${PROJECT_BINARY_DIR}/foo.c
  )

# Add custom command to generate not_included.h, which is a header
# file that is not included by any source in this project.  This will
# test whether all custom command outputs explicitly listed as sources
# get generated even if they are not needed by an object file.
ADD_CUSTOM_COMMAND(OUTPUT ${PROJECT_BINARY_DIR}/not_included.h
  DEPENDS ${PROJECT_SOURCE_DIR}/foo.h.in
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/foo.h.in
                                   ${PROJECT_BINARY_DIR}/not_included.h
  )

# Tell the executable where to find not_included.h.
CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/config.h.in
  ${PROJECT_BINARY_DIR}/config.h
  @ONLY IMMEDIATE
  )

# add the executable
ADD_EXECUTABLE(CustomCommand 
  ${PROJECT_BINARY_DIR}/foo.h
  ${PROJECT_BINARY_DIR}/foo.c
  ${PROJECT_BINARY_DIR}/wrapped.c
  ${PROJECT_BINARY_DIR}/wrapped_help.c
  ${PROJECT_BINARY_DIR}/generated.c
  ${PROJECT_BINARY_DIR}/not_included.h
  gen_redirect.c # default location for custom commands is in build tree
  )

# Add the rule to create generated.c at build time.  This is placed
# here to test adding the generation rule after referencing the
# generated source in a target.
ADD_CUSTOM_COMMAND(OUTPUT ${PROJECT_BINARY_DIR}/generated.c
  DEPENDS generator
  COMMAND ${generator_PATH}
  ARGS ${PROJECT_BINARY_DIR}/generated.c
  )

TARGET_LINK_LIBRARIES(CustomCommand GeneratedHeader)

# must add a dependency on TDocument otherwise it might never build and 
# the CustomCommand executable really needs doc1.h
ADD_DEPENDENCIES(CustomCommand TDocument)

##############################################################################
# Test for using just the target name as executable in the COMMAND
# section. Has to be recognized and replaced by CMake with the output
# actual location of the executable.
# Additionally the generator is created in an extra subdir after the 
# ADD_CUSTOM_COMMAND() is used.
#
# Test the same for ADD_CUSTOM_TARGET()

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated_extern.cxx
  COMMAND generator_extern ${CMAKE_CURRENT_BINARY_DIR}/generated_extern.cxx
  )

ADD_EXECUTABLE(CustomCommandUsingTargetTest main.cxx ${CMAKE_CURRENT_BINARY_DIR}/generated_extern.cxx )

ADD_CUSTOM_TARGET(RunTarget 
  COMMAND generator_extern ${CMAKE_CURRENT_BINARY_DIR}/run_target.cxx
  )

ADD_CUSTOM_COMMAND(TARGET CustomCommandUsingTargetTest POST_BUILD 
                   COMMAND dummy_generator ${CMAKE_CURRENT_BINARY_DIR}/generated_dummy.cxx)

ADD_SUBDIRECTORY(GeneratorInExtraDir)

##############################################################################
# Test shell operators in custom commands.

ADD_EXECUTABLE(tcat tcat.cxx)

ADD_CUSTOM_COMMAND(OUTPUT gen_redirect.c
  DEPENDS tcat gen_redirect_in.c
  COMMAND tcat < ${CMAKE_CURRENT_SOURCE_DIR}/gen_redirect_in.c > gen_redirect.c
  COMMAND ${CMAKE_COMMAND} -E echo "#endif" >> gen_redirect.c
  VERBATIM
  )

##############################################################################
# Test non-trivial command line arguments in custom commands.
SET(EXPECTED_ARGUMENTS)
SET(CHECK_ARGS
  c:/posix/path
  c:\\windows\\path
  'single-quotes'
  single'quote
  \"double-quotes\"
  "\\;semi-colons\\;"
  "semi\\;colon"
  `back-ticks`
  back`tick
  "(parens)"
  "(lparen"
  "rparen)"
  {curly}
  {lcurly}
  rcurly}
  <angle>
  <langle
  rangle>
  [square]
  [lsquare # these have funny behavior due to special cases for
  rsquare] # windows registry value names in list expansion
  $dollar-signs$
  dollar$sign
  &ampersands&x # Borland make does not like trailing ampersand
  one&ampersand
  @two-ats@
  one@at
  ~two-tilda~
  one~tilda
  ^two-carrots^
  one^carrot
  %two-percents%
  one%percent
  !two-exclamations!
  one!exclamation
  ?two-questions?
  one?question
  *two-stars*
  one*star
  =two+equals=
  one=equals
  _two-underscores_
  one_underscore
  ,two-commas,
  one,comma
  .two-periods.
  one.period
  |two-pipes|
  one|pipe
  |nopipe
  "#two-pounds#"
  "one#pound"
  "#nocomment"
  "c:/posix/path/with space"
  "c:\\windows\\path\\with space"
  "'single quotes with space'"
  "single'quote with space"
  "\"double-quotes with space\""
  "\\;semi-colons w s\\;"
  "semi\\;colon w s"
  "`back-ticks` w s"
  "back`tick w s"
  "(parens) w s"
  "(lparen w s"
  "rparen) w s"
  "{curly} w s"
  "{lcurly w s"
  "rcurly} w s"
  "<angle> w s"
  "<langle w s"
  "rangle> w s"
  "[square] w s"
  "[lsquare w s" # these have funny behavior due to special cases for
  "rsquare] w s" # windows registry value names in list expansion
  "$dollar-signs$ w s"
  "dollar$sign w s"
  "&ampersands& w s"
  "one&ampersand w s"
  "@two-ats@ w s"
  "one@at w s"
  "~two-tilda~ w s"
  "one~tilda w s"
  "^two-carrots^ w s"
  "one^carrot w s"
  "%two-percents% w s"
  "one%percent w s"
  "!two-exclamations! w s"
  "one!exclamation w s"
  "*two-stars* w s"
  "one*star w s"
  "=two+equals= w s"
  "one=equals w s"
  "_two-underscores_ w s"
  "one_underscore w s"
  "?two-questions? w s"
  "one?question w s"
  ",two-commas, w s"
  "one,comma w s"
  ".two-periods. w s"
  "one.period w s"
  "|two-pipes| w s"
  "one|pipe w s"
  "#two-pounds# w s"
  "one#pound w s"
  ~ ` ! @ \# $ % ^ & _ - + = : "\;" \" ' , . ? "(" ")" { } []
  )
IF(NOT MINGW)
  # *  # MinGW programs on windows always expands the wildcard!
  # /  # MSys make converts a leading slash to the mingw home directory
  LIST(APPEND CHECK_ARGS * /)
ENDIF(NOT MINGW)

# The windows command shell does not support a double quote by itself:
#   double\"quote
# without messing up quoting of arguments following it.

# Make tools need help with escaping a single backslash
#   \
# at the end of a command because they think it is a continuation
# character.

# We now have special cases for shell operators:
#   | < > << >> &> 2>&1 1>&2
# to allow custom commands to perform redirection.

FOREACH(arg ${CHECK_ARGS} "")
  SET(ARG "${arg}")
  STRING(REGEX REPLACE "\\\\" "\\\\\\\\" ARG "${ARG}")
  STRING(REGEX REPLACE "\"" "\\\\\"" ARG "${ARG}")
  SET(EXPECTED_ARGUMENTS
    "${EXPECTED_ARGUMENTS}  \"${ARG}\",
")
ENDFOREACH(arg)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/check_command_line.c.in
               ${CMAKE_CURRENT_BINARY_DIR}/check_command_line.c
               @ONLY IMMEDIATE)
ADD_EXECUTABLE(check_command_line
  ${CMAKE_CURRENT_BINARY_DIR}/check_command_line.c)
# SET_TARGET_PROPERTIES(check_command_line PROPERTIES
#   COMPILE_FLAGS -DCHECK_COMMAND_LINE_VERBOSE)
ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/command_line_check
  COMMAND ${CMAKE_COMMAND} -DMARK_FILE=${CMAKE_CURRENT_BINARY_DIR}/check_mark.txt
  -P ${CMAKE_CURRENT_SOURCE_DIR}/check_mark.cmake
  COMMAND ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/check_command_line
  ${CHECK_ARGS} ""
  VERBATIM
  COMMENT "Checking custom command line escapes (single'quote)"
  )
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/command_line_check
  PROPERTIES SYMBOLIC 1)
ADD_CUSTOM_TARGET(do_check_command_line ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/command_line_check
  COMMAND ${CMAKE_COMMAND} -E echo "Checking custom target command escapes"
  COMMAND ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/check_command_line
  ${CHECK_ARGS} ""
  VERBATIM
  COMMENT "Checking custom target command line escapes ($dollar-signs$)"
  )
ADD_DEPENDENCIES(do_check_command_line check_command_line)

ADD_CUSTOM_TARGET(pre_check_command_line
  COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/check_mark.txt
  )
ADD_DEPENDENCIES(do_check_command_line pre_check_command_line)

# <SameNameTest>
#
# Add a custom target called "SameName" -- then add a custom command in a
# different target whose output is a full-path file called "SameName" -- then
# add a second custom target that depends on the full-path file ".../SameName"
#
# At first, this reproduces a bug reported by a customer. After fixing it,
# having this test here makes sure it stays fixed moving forward.
#
ADD_CUSTOM_COMMAND(
  OUTPUT SameName1.txt
  COMMAND ${CMAKE_COMMAND} -E touch SameName1.txt
  )
ADD_CUSTOM_TARGET(SameName ALL
  DEPENDS SameName1.txt
  )

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/subdir/SameName
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/subdir
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/subdir/SameName
  )
ADD_CUSTOM_TARGET(DifferentName ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/subdir/SameName
  )
#
# </SameNameTest>
