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
             NAMES mpich2 mpi mpich mpich.rts
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

# on BlueGene/L the MPI lib is named libmpich.rts.a, there also these additional libs are required
IF("${MPI_LIBRARY}" MATCHES "mpich.rts")
   SET(MPI_EXTRA_LIBRARY msglayer.rts devices.rts rts.rts devices.rts CACHE STRING "Additional MPI libs" FORCE)
ENDIF("${MPI_LIBRARY}" MATCHES "mpich.rts")

INCLUDE(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MPI DEFAULT_MSG MPI_LIBRARY MPI_INCLUDE_PATH)

MARK_AS_ADVANCED(MPI_INCLUDE_PATH MPI_LIBRARY MPI_EXTRA_LIBRARY)
