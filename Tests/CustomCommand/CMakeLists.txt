#
# Wrapping
#
PROJECT (CustomCommand)

#
# Lib and exe path
#
SET (LIBRARY_OUTPUT_PATH 
     ${PROJECT_BINARY_DIR}/bin/ CACHE PATH 
     "Single output directory for building all libraries.")

SET (EXECUTABLE_OUTPUT_PATH 
     ${PROJECT_BINARY_DIR}/bin/ CACHE PATH 
     "Single output directory for building all executables.")

################################################################
#
#  First test using a compiled generator to create a .c file
#
################################################################
# add the executable that will generate the file
ADD_EXECUTABLE(generator generator.c)

# the folowing assumes that a cmSourceFile
# is instantiated for the output, with GENERATED 1
# at the end of the day this becomes a what in VS ?
ADD_CUSTOM_COMMAND(OUTPUT ${PROJECT_BINARY_DIR}/generated.c
      DEPENDS generator
      COMMAND ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/generator
      ARGS ${PROJECT_BINARY_DIR}/generated.c
      )

################################################################
#
#  Test using a wrapper to wrap a header file
#
################################################################
# add the executable that will generate the file
ADD_EXECUTABLE(wrapper wrapper.c)

# the following assumes that a cmSourceFile
# is instantiated for the output, with GENERATED 1
# at the end of the day this becomes a what in VS ?
ADD_CUSTOM_COMMAND(
      OUTPUT ${PROJECT_BINARY_DIR}/wrapped.c
      DEPENDS wrapper
      MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/wrapped.h
      COMMAND ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/wrapper
      ARGS ${PROJECT_BINARY_DIR}/wrapped.c ${PROJECT_SOURCE_DIR}/wrapped.h
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
      DEPENDS   ${PROJECT_BINARY_DIR}/doc1.dvi 
      COMMAND   ${CMAKE_COMMAND}
      ARGS      -E copy ${PROJECT_BINARY_DIR}/doc1.dvi
                        ${PROJECT_BINARY_DIR}/doc1.h
      )

ADD_CUSTOM_TARGET(TDocument ALL 
      ${CMAKE_COMMAND} -E echo "building doc1.h"  
      DEPENDS ${PROJECT_BINARY_DIR}/doc1.h
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
      DEPENDS   TDocument ${PROJECT_BINARY_DIR}/foo.pre 
      COMMAND   ${CMAKE_COMMAND}
      ARGS      -E copy ${PROJECT_BINARY_DIR}/foo.pre
                        ${PROJECT_BINARY_DIR}/foo.c
      )

# add the library
ADD_EXECUTABLE(CustomCommand 
  ${PROJECT_BINARY_DIR}/foo.c
  ${PROJECT_BINARY_DIR}/wrapped.c
  ${PROJECT_BINARY_DIR}/generated.c
  )

# must add a dependency on TDocument otherwise it might never build and 
# the CustomCommand executable really needs doc1.h
ADD_DEPENDENCIES(CustomCommand TDocument)