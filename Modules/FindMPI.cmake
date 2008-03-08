# This module looks for the Message Passing Interface (MPI).
#
# This module will set the following variables:
#   MPI_FOUND                  TRUE if we have found MPI
#   MPI_COMPILE_FLAGS          Compilation flags for MPI programs
#   MPI_INCLUDE_PATH           Include path for MPI header
#   MPI_LINK_FLAGS             Linking flags for MPI programs
#   MPI_LIBRARIES              Libraries to link MPI programs against
#   MPI_LIBRARY                Deprecated; first MPI library to link against
#   MPI_EXTRA_LIBRARY          Deprecated; second MPI library to link against
#   MPIEXEC                    Executable for running MPI programs
#
# This module will attempt to auto-detect these settings, first by
# looking for a C++ MPI driver (e.g., mpic++, mpiCC, or mpicxx; set by
# MPICXX) and later by checking common MPI paths and library names.

# Try to find the MPI driver program
find_program(MPICXX 
  NAMES mpic++ mpicxx mpiCC
  DOC "MPI C++ compiler. Used only to detect MPI compilation flags.")
mark_as_advanced(MPICXX)

find_program(MPIEXEC
  NAMES mpiexec mpirun
  DOC "Executable for running MPI programs.")
mark_as_advanced(MPIEXEC)

if (NOT OLD_MPICXX STREQUAL MPICXX)
  set(MPI_FORCE_RECONFIGURE TRUE)
  set(OLD_MPICXX ${MPICXX} CACHE INTERNAL "Previous value of MPICXX" FORCE)
endif (NOT OLD_MPICXX STREQUAL MPICXX)
  
if (NOT MPICXX)
  # If there is no MPI C++ compiler, we force ourselves to configure
  # MPI the old way.
  set(MPI_FORCE_RECONFIGURE TRUE)
endif (NOT MPICXX)

if (MPICXX)
  # Check whether the -showme:compile option works. This indicates
  # that we have either Open MPI or a newer version of LAM-MPI, and
  # implies that -showme:link will also work.
  exec_program(${MPICXX} 
    ARGS -showme:compile 
    OUTPUT_VARIABLE MPI_COMPILE_CMDLINE
    RETURN_VALUE MPICXX_RETURN)

  if (MPICXX_RETURN EQUAL 0)
    # If we appear to have -showme:compile, then we should also have
    # -showme:link. Try it.
    exec_program(${MPICXX} 
      ARGS -showme:link
      OUTPUT_VARIABLE MPI_LINK_CMDLINE
      RETURN_VALUE MPICXX_RETURN)
  endif (MPICXX_RETURN EQUAL 0)

  if (MPICXX_RETURN EQUAL 0)
    # Do nothing: we have our command lines now
  else (MPICXX_RETURN EQUAL 0)
    # Older versions of LAM-MPI have "-showme". Try it.
    exec_program(${MPICXX} 
      ARGS -showme
      OUTPUT_VARIABLE MPI_COMPILE_CMDLINE
      RETURN_VALUE MPICXX_RETURN)
  endif (MPICXX_RETURN EQUAL 0)  

  if (MPICXX_RETURN EQUAL 0)
    # Do nothing: we have our command lines now
  else (MPICXX_RETURN EQUAL 0)
    # MPICH uses "-show". Try it.
    exec_program(${MPICXX} 
      ARGS -show
      OUTPUT_VARIABLE MPI_COMPILE_CMDLINE
      RETURN_VALUE MPICXX_RETURN)
  endif (MPICXX_RETURN EQUAL 0)  

  if (MPICXX_RETURN EQUAL 0)
    # We have our command lines, but we might need to copy
    # MPI_COMPILE_CMDLINE into MPI_LINK_CMDLINE, if the underlying
    if (NOT MPI_LINK_CMDLINE)
      SET(MPI_LINK_CMDLINE ${MPI_COMPILE_CMDLINE})
    endif (NOT MPI_LINK_CMDLINE)
  else (MPICXX_RETURN EQUAL 0)
    message(STATUS "Unable to determine MPI from MPI driver ${MPICXX}")
  endif (MPICXX_RETURN EQUAL 0)
endif (MPICXX)

if (NOT MPI_FORCE_RECONFIGURE)
  # We don't actually have to reconfigure anything
elseif (MPI_COMPILE_CMDLINE)
  # Extract compile flags from the compile command line.
  string(REGEX MATCHALL "-D([^\" ]+|\"[^\"]+\")" MPI_ALL_COMPILE_FLAGS ${MPI_COMPILE_CMDLINE})
  set(MPI_COMPILE_FLAGS_WORK)
  foreach(FLAG ${MPI_ALL_COMPILE_FLAGS})
    if (MPI_COMPILE_FLAGS_WORK)
      set(MPI_COMPILE_FLAGS_WORK "${MPI_COMPILE_FLAGS_WORK} ${FLAG}")
    else(MPI_COMPILE_FLAGS_WORK)
      set(MPI_COMPILE_FLAGS_WORK ${FLAG})
    endif(MPI_COMPILE_FLAGS_WORK)
  endforeach(FLAG)

  # Extract include paths from compile command line
  string(REGEX MATCH "-I([^\" ]+|\"[^\"]+\")" MPI_INCLUDE_PATH ${MPI_COMPILE_CMDLINE})
  string(REGEX REPLACE "^-I" "" MPI_INCLUDE_PATH ${MPI_INCLUDE_PATH})
  string(REGEX REPLACE "//" "/" MPI_INCLUDE_PATH ${MPI_INCLUDE_PATH})

  # Extract linker paths from the link command line
  string(REGEX MATCH "-L([^\" ]+|\"[^\"]+\")" MPI_LINK_PATH ${MPI_LINK_CMDLINE})
  string(REGEX REPLACE "^-L" "" MPI_LINK_PATH ${MPI_LINK_PATH})
  string(REGEX REPLACE "//" "/" MPI_LINK_PATH ${MPI_LINK_PATH})

  # Extract linker flags from the link command line
  string(REGEX MATCHALL "-Wl,([^\" ]+|\"[^\"]+\")" MPI_ALL_LINK_FLAGS ${MPI_LINK_CMDLINE})
  set(MPI_LINK_FLAGS_WORK)
  foreach(FLAG ${MPI_ALL_LINK_FLAGS})
    if (MPI_LINK_FLAGS_WORK)
      set(MPI_LINK_FLAGS_WORK "${MPI_LINK_FLAGS_WORK} ${FLAG}")
    else(MPI_LINK_FLAGS_WORK)
      set(MPI_LINK_FLAGS_WORK ${FLAG})
    endif(MPI_LINK_FLAGS_WORK)
  endforeach(FLAG)

  # Extract the set of libraries to link against from the link command
  # line
  string(REGEX MATCHALL "-l([^\" ]+|\"[^\"]+\")" MPI_LIBNAMES ${MPI_LINK_CMDLINE})

  # Determine full path names for all of the libraries that one needs
  # to link against in an MPI program
  set(MPI_LIBRARIES)
  foreach(LIB ${MPI_LIBNAMES})
    string(REGEX REPLACE "^-l" "" LIB ${LIB})
    set(MPI_LIB "MPI_LIB-NOTFOUND" CACHE FILEPATH "Cleared" FORCE)
    find_library(MPI_LIB ${LIB} PATHS ${MPI_LINK_PATH})
    if (MPI_LIB)
      list(APPEND MPI_LIBRARIES ${MPI_LIB})
    else (MPI_LIB)
      status(ERROR "Unable to find MPI library ${LIB}")
    endif (MPI_LIB)
  endforeach(LIB)
  set(MPI_LIB "MPI_LIB-NOTFOUND" CACHE INTERNAL "Scratch variable for MPI detection" FORCE)

  # Chop MPI_LIBRARIES into the old-style MPI_LIBRARY and
  # MPI_EXTRA_LIBRARY.
  list(LENGTH MPI_LIBRARIES MPI_NUMLIBS)
  if (MPI_NUMLIBS GREATER 0)
    list(GET MPI_LIBRARIES 0 MPI_LIBRARY)
  else (MPI_NUMLIBS GREATER 0)
    set(MPI_LIBRARY "MPI_LIBRARY-NOTFOUND")
  endif (MPI_NUMLIBS GREATER 0)
  if (MPI_NUMLIBS GREATER 1)
    cdr(MPI_EXTRA_LIBRARY ${MPI_LIBRARIES})
  else (MPI_NUMLIBS GREATER 1)
    set(MPI_EXTRA_LIBRARY "MPI_EXTRA_LIBRARY-NOTFOUND")
  endif (MPI_NUMLIBS GREATER 1)

  # Set up all of the appropriate cache entries
  set(MPI_FOUND TRUE CACHE INTERNAL "Whether MPI was found" FORCE)

  if (NOT MPI_FORCE_RECONFIGURE)
    set(MPI_COMPILE_FLAGS ${MPI_COMPILE_FLAGS_WORK} CACHE STRING "MPI compilation flags")
    set(MPI_INCLUDE_PATH ${MPI_INCLUDE_PATH} CACHE STRING "MPI include path")
    set(MPI_LINK_FLAGS ${MPI_LINK_FLAGS_WORK} CACHE STRING "MPI linking flags")
    set(MPI_LIBRARIES ${MPI_LIBRARIES} CACHE STRING "MPI libraries to link against, separated by semicolons")
  else (NOT MPI_FORCE_RECONFIGURE)
    set(MPI_COMPILE_FLAGS ${MPI_COMPILE_FLAGS_WORK} CACHE STRING "MPI compilation flags" FORCE)
    set(MPI_INCLUDE_PATH ${MPI_INCLUDE_PATH} CACHE STRING "MPI include path" FORCE)
    set(MPI_LINK_FLAGS ${MPI_LINK_FLAGS_WORK} CACHE STRING "MPI linking flags" FORCE)
    set(MPI_LIBRARIES ${MPI_LIBRARIES} CACHE STRING "MPI libraries to link against, separated by semicolons" FORCE)
  endif (NOT MPI_FORCE_RECONFIGURE)
else (MPI_COMPILE_CMDLINE)
  find_path(MPI_INCLUDE_PATH mpi.h 
    /usr/local/include 
    /usr/include 
    /usr/include/mpi
    /usr/local/mpi/include
    "C:/Program Files/MPICH/SDK/Include" 
    "$ENV{SystemDrive}/Program Files/MPICH2/include"
    "C:/Program Files/Microsoft Compute Cluster Pack/Include"
    )
  
  # TODO: How do we know whether we're building 32-bit vs. 64-bit?
  find_library(MPI_LIBRARY 
    NAMES mpi mpich
    PATHS /usr/lib /usr/local/lib /usr/local/mpi/lib
    "C:/Program Files/MPICH/SDK/Lib" 
    "$ENV{SystemDrive}/Program Files/MPICH/SDK/Lib"
    "C:/Program Files/Microsoft Compute Cluster Pack/Lib/i386"
    )
  find_library(MPI_LIBRARY 
    NAMES mpich2
    PATHS
    "$ENV{SystemDrive}/Program Files/MPICH2/Lib")

  find_library(MPI_EXTRA_LIBRARY 
    NAMES mpi++
    PATHS /usr/lib /usr/local/lib /usr/local/mpi/lib
    "C:/Program Files/MPICH/SDK/Lib" 
    "C:/Program Files/Microsoft Compute Cluster Pack/Lib/i386"
    DOC "If a second MPI library is necessary, specify it here.")

  set(MPI_COMPILE_FLAGS "" CACHE STRING "MPI compilation flags")
  set(MPI_LINK_FLAGS "" CACHE STRING "MPI linking flags")

  if (MPI_EXTRA_LIBRARY)
    set(MPI_LIBRARIES "${MPI_LIBRARY};${MPI_EXTRA_LIBRARY}" CACHE STRING "MPI libraries to link against, separated by semicolons")
  else (MPI_EXTRA_LIBRARY)
    set(MPI_LIBRARIES ${MPI_LIBRARY} CACHE STRING "MPI libraries to link against, separated by semicolons")
  endif (MPI_EXTRA_LIBRARY)

  if (MPI_LIBRARY)
    set(MPI_FOUND TRUE CACHE INTERNAL "Whether MPI was found" FORCE)
  else (MPI_LIBRARY)
    set(MPI_FOUND FALSE CACHE INTERNAL "Whether MPI was found" FORCE)
  endif (MPI_LIBRARY)
endif (NOT MPI_FORCE_RECONFIGURE)

# on BlueGene/L the MPI lib is named libmpich.rts.a, there also these additional libs are required
if("${MPI_LIBRARY}" MATCHES "mpich.rts")
   set(MPI_EXTRA_LIBRARY ${MPI_EXTRA_LIBRARY} msglayer.rts devices.rts rts.rts devices.rts)
   set(MPI_LIBRARY ${MPI_LIBRARY}  msglayer.rts devices.rts rts.rts devices.rts)
endif("${MPI_LIBRARY}" MATCHES "mpich.rts")

set(MPI_LIBRARY ${MPI_LIBRARY} CACHE INTERNAL "MPI library to link against. Deprecated: use MPI_LIBRARIES instead")
set(MPI_EXTRA_LIBRARY ${MPI_EXTRA_LIBRARY} CACHE INTERNAL "Second MPI library to link against. Deprecated: use MPI_LIBRARIES instead")

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments 
find_package_handle_standard_args(MPI DEFAULT_MSG MPI_LIBRARY MPI_INCLUDE_PATH)
mark_as_advanced(MPI_INCLUDE_PATH MPI_COMPILE_FLAGS MPI_LINK_FLAGS MPI_LIBRARIES)
