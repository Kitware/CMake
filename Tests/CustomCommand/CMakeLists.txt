#
# Wrapping
#
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

# the folowing assumes that a cmSourceFile
# is instantiated for the output, with GENERATED 1
# at the end of the day this becomes a what in VS ?
ADD_CUSTOM_COMMAND(OUTPUT ${PROJECT_BINARY_DIR}/generated.c
  DEPENDS generator
  COMMAND ${generator_PATH}
  ARGS ${PROJECT_BINARY_DIR}/generated.c
  )

################################################################
#
#  Test using a wrapper to wrap a header file
#
################################################################
# add the executable that will generate the file
ADD_EXECUTABLE(wrapper wrapper.cxx)

# the following assumes that a cmSourceFile
# is instantiated for the output, with GENERATED 1
# at the end of the day this becomes a what in VS ?
ADD_CUSTOM_COMMAND(
  OUTPUT ${PROJECT_BINARY_DIR}/wrapped.c ${PROJECT_BINARY_DIR}/wrapped_help.c
  DEPENDS wrapper
  MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/wrapped.h
  COMMAND ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/wrapper
  ${PROJECT_BINARY_DIR}/wrapped.c ${PROJECT_BINARY_DIR}/wrapped_help.c
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
  DEPENDS ${PROJECT_BINARY_DIR}/doc1.dvi
  COMMAND ${CMAKE_COMMAND} -E echo " Copying doc1.dvi to doc1temp.h."
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/doc1.dvi
                                   ${PROJECT_BINARY_DIR}/doc1temp.h
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

# Add a custom target to drive generation of doc1.h.
ADD_CUSTOM_TARGET(TDocument ALL
  COMMAND ${CMAKE_COMMAND} -E echo " Copying doc1.h to doc2.h."
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/doc1.h
                                   ${PROJECT_BINARY_DIR}/doc2.h
  DEPENDS ${PROJECT_BINARY_DIR}/doc1.h
  )

# Setup a pre- and post-build pair that will fail if not run in the
# proper order.
ADD_CUSTOM_COMMAND(
  TARGET TDocument PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo " Writing doc1pre.txt."
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/doc1.tex ${PROJECT_BINARY_DIR}/doc1pre.txt
  )
ADD_CUSTOM_COMMAND(
  TARGET TDocument POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo " Copying doc1pre.txt to doc2post.txt."
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/doc1pre.txt
                                   ${PROJECT_BINARY_DIR}/doc2post.txt
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
  )

TARGET_LINK_LIBRARIES(CustomCommand GeneratedHeader)

# must add a dependency on TDocument otherwise it might never build and 
# the CustomCommand executable really needs doc1.h
ADD_DEPENDENCIES(CustomCommand TDocument)

