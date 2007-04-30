# - Find MPI
# This module looks for MPI (Message Passing Interface) support
# it will define the following values
#  MPI_INCLUDE_PATH = where mpi.h can be found
#  MPI_LIBRARY    = the library to link in (mpi mpich etc)

FIND_PATH(MPI_INCLUDE_PATH NAMES mpi.h 
          PATH_SUFFIXES mpi mpi/include
          PATHS
          "$ENV{ProgramFiles}/MPICH/SDK/Include"
          "$ENV{ProgramFiles}/MPICH2/include"
          "C:/Program Files/MPICH/SDK/Include" 
)

FIND_LIBRARY(MPI_LIBRARY 
             NAMES mpich2 mpi mpich 
             PATH_SUFFIXES mpi/lib
             PATHS
             "$ENV{ProgramFiles}/MPICH/SDK/Lib"
             "$ENV{ProgramFiles}/MPICH2/Lib"
             "C:/Program Files/MPICH/SDK/Lib" 
)

FIND_LIBRARY(MPI_EXTRA_LIBRARY 
             NAMES mpi++
             PATH_SUFFIXES mpi/lib
             PATHS
             "$ENV{ProgramFiles}/MPICH/SDK/Lib"
             "C:/Program Files/MPICH/SDK/Lib" 
             DOC "If a second mpi library is necessary, specify it here.")

MARK_AS_ADVANCED(MPI_INCLUDE_PATH MPI_LIBRARY MPI_EXTRA_LIBRARY)
