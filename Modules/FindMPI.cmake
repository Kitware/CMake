#
# this module look sfor MPI (Message Passing Interface) support
# it will define the following values
#
# MPI_INCLUDE_PATH = where mpi.h can be found
# MPI_LIBRARY      = the library to link against (mpi mpich etc)
#

FIND_PATH(MPI_INCLUDE_PATH mpi.h /usr/local/include /usr/include /usr/local/mpi/include)

FIND_LIBRARY(MPI_LIBRARY 
             NAMES mpi mpich
             PATHS /usr/lib /usr/local/lib /usr/local/mpi/lib)

