#
# this module look sfor MPI (Message Passing Interface) support
# it will define the following values
#
# MPI_INCLUDE_PATH = where mpi.h can be found
# MPI_LIBRARY      = the library to link against (mpi mpich etc)
#

FIND_PATH(MPI_INCLUDE_PATH mpi.h 
          /usr/local/include 
          /usr/include 
          /usr/include/mpi
          /usr/local/mpi/include)

FIND_LIBRARY(MPI_LIBRARY 
             NAMES mpi mpich
             PATHS /usr/lib /usr/local/lib /usr/local/mpi/lib)

FIND_LIBRARY(MPI_EXTRA_LIBRARY 
             NAMES mpi++
             PATHS /usr/lib /usr/local/lib /usr/local/mpi/lib
	     DOC "If a second mpi library is necessary, specify it here.")

MARK_AS_ADVANCED(MPI_INCLUDE_PATH MPI_LIBRARY MPI_EXTRA_LIBRARY)
