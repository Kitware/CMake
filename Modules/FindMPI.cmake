#
# this module look sfor MPI (Message Passing Interface) support
# it will define the following values
#
# MPI_INCLUDE_PATH = where mpi.h can be found
# MPI_LIB_PATH     = path to the mpi library
# MPI_LIBRARY      = the library to link against (mpi mpich etc)
#

FIND_INCLUDE(MPI_INCLUDE_PATH mpi.h /usr/local/include /usr/include)

# look for the different MPI libs
IF (NOT MPI_LIB_PATH)
  FIND_LIBRARY(MPI_LIB_PATH mpi /usr/lib /usr/local/lib)
  IF (MPI_LIB_PATH)
    SET (MPI_LIBRARY mpi CACHE)
  ENDIF (MPI_LIB_PATH)
ENDIF (NOT MPI_LIB_PATH)
IF (NOT MPI_LIB_PATH)
  FIND_LIBRARY(MPI_LIB_PATH mpich /usr/lib /usr/local/lib)
  IF (MPI_LIB_PATH)
    SET (MPI_LIBRARY mpich CACHE)
  ENDIF (MPI_LIB_PATH)
ENDIF (NOT MPI_LIB_PATH)

