# this file contains the most basic rules.  This is used 
# in the CMake/Source/Makefile.in to avoid infinite recursion
# in the rule for all:

# set up make suffixes

.SUFFIXES: .cxx .java .class

#------------------------------------------------------------------------------
# rules for building .o files from source files

.c.o:
	${CC} ${CC_FLAGS} -c $< -o $@
.cxx.o:
	${CXX} ${CXX_FLAGS} -c $< -o $@
