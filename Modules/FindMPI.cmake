# - Message Passing Interface (MPI) module.
# 
# The Message Passing Interface (MPI) is a library used to write
# high-performance parallel applications that use message passing, and
# is typically deployed on a cluster. MPI is a standard interface
# (defined by the MPI forum) for which many implementations are
# available. All of these implementations have somewhat different
# compilation approaches (different include paths, libraries to link
# against, etc.), and this module tries to smooth out those differences.
#
# This module will set the following variables:
#   MPI_FOUND                  TRUE if we have found MPI
#   MPI_COMPILE_FLAGS          Compilation flags for MPI programs
#   MPI_INCLUDE_PATH           Include path(s) for MPI header
#   MPI_LINK_FLAGS             Linking flags for MPI programs
#   MPI_LIBRARY                First MPI library to link against (cached)
#   MPI_EXTRA_LIBRARY          Extra MPI libraries to link against (cached)
#   MPI_LIBRARIES              All libraries to link MPI programs against
#   MPIEXEC                    Executable for running MPI programs
#   MPIEXEC_NUMPROC_FLAG       Flag to pass to MPIEXEC before giving it the
#                              number of processors to run on
#   MPIEXEC_PREFLAGS           Flags to pass to MPIEXEC directly before the
#                              executable to run.
#   MPIEXEC_POSTFLAGS          Flags to pass to MPIEXEC after all other flags.
#
# This module will attempt to auto-detect these settings, first by
# looking for a MPI compiler, which many MPI implementations provide
# as a pass-through to the native compiler to simplify the compilation
# of MPI programs. The MPI compiler is stored in the cache variable
# MPI_COMPILER, and will attempt to look for commonly-named drivers
# mpic++, mpicxx, mpiCC, or mpicc. If the compiler driver is found and
# recognized, it will be used to set all of the module variables. To
# skip this auto-detection, set MPI_LIBRARY and MPI_INCLUDE_PATH in
# the CMake cache.
#
# If no compiler driver is found or the compiler driver is not
# recognized, this module will then search for common include paths
# and library names to try to detect MPI. 
#
# If CMake initially finds a different MPI than was intended, and you
# want to use the MPI compiler auto-detection for a different MPI
# implementation, set MPI_COMPILER to the MPI compiler driver you want
# to use (e.g., mpicxx) and then set MPI_LIBRARY to the string
# MPI_LIBRARY-NOTFOUND. When you re-configure, auto-detection of MPI
# will run again with the newly-specified MPI_COMPILER.
#
# When using MPIEXEC to execute MPI applications, you should typically
# use all of the MPIEXEC flags as follows:
#   ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} PROCS ${MPIEXEC_PREFLAGS} EXECUTABLE
#     ${MPIEXEC_POSTFLAGS} ARGS
# where PROCS is the number of processors on which to execute the program,
# EXECUTABLE is the MPI program, and ARGS are the arguments to pass to the 
# MPI program.

# Try to find the MPI driver program
find_program(MPI_COMPILER 
  NAMES mpic++ mpicxx mpiCC mpicc
  DOC "MPI compiler. Used only to detect MPI compilation flags.")
mark_as_advanced(MPI_COMPILER)

find_program(MPIEXEC
  NAMES mpiexec mpirun lamexec
  DOC "Executable for running MPI programs.")

set(MPIEXEC_NUMPROC_FLAG "-np" CACHE STRING "Flag used by MPI to specify the number of processes for MPIEXEC; the next option will be the number of processes.")
set(MPIEXEC_PREFLAGS "" CACHE STRING "These flags will be directly before the executable that is being run by MPIEXEC.")
set(MPIEXEC_POSTFLAGS "" CACHE STRING "These flags will come after all flags given to MPIEXEC.")
set(MPIEXEC_MAX_NUMPROCS "2" CACHE STRING "Maximum number of processors available to run MPI applications.")
mark_as_advanced(MPIEXEC MPIEXEC_NUMPROC_FLAG MPIEXEC_PREFLAGS 
  MPIEXEC_POSTFLAGS MPIEXEC_MAX_NUMPROCS)

if (MPI_INCLUDE_PATH AND MPI_LIBRARY)
  # Do nothing: we already have MPI_INCLUDE_PATH and MPI_LIBRARY in
  # the cache, and we don't want to override those settings.
elseif (MPI_COMPILER)
  # Check whether the -showme:compile option works. This indicates
  # that we have either Open MPI or a newer version of LAM-MPI, and
  # implies that -showme:link will also work.
  exec_program(${MPI_COMPILER} 
    ARGS -showme:compile 
    OUTPUT_VARIABLE MPI_COMPILE_CMDLINE
    RETURN_VALUE MPI_COMPILER_RETURN)

  if (MPI_COMPILER_RETURN EQUAL 0)
    # If we appear to have -showme:compile, then we should also have
    # -showme:link. Try it.
    exec_program(${MPI_COMPILER} 
      ARGS -showme:link
      OUTPUT_VARIABLE MPI_LINK_CMDLINE
      RETURN_VALUE MPI_COMPILER_RETURN)

    # Note that we probably have -showme:incdirs and -showme:libdirs
    # as well.
    set(MPI_COMPILER_MAY_HAVE_INCLIBDIRS TRUE)
  endif (MPI_COMPILER_RETURN EQUAL 0)

  if (MPI_COMPILER_RETURN EQUAL 0)
    # Do nothing: we have our command lines now
  else (MPI_COMPILER_RETURN EQUAL 0)
    # Older versions of LAM-MPI have "-showme". Try it.
    exec_program(${MPI_COMPILER} 
      ARGS -showme
      OUTPUT_VARIABLE MPI_COMPILE_CMDLINE
      RETURN_VALUE MPI_COMPILER_RETURN)
  endif (MPI_COMPILER_RETURN EQUAL 0)  

  if (MPI_COMPILER_RETURN EQUAL 0)
    # Do nothing: we have our command lines now
  else (MPI_COMPILER_RETURN EQUAL 0)
    # MPICH uses "-show". Try it.
    exec_program(${MPI_COMPILER} 
      ARGS -show
      OUTPUT_VARIABLE MPI_COMPILE_CMDLINE
      RETURN_VALUE MPI_COMPILER_RETURN)
  endif (MPI_COMPILER_RETURN EQUAL 0)  

  if (MPI_COMPILER_RETURN EQUAL 0)
    # We have our command lines, but we might need to copy
    # MPI_COMPILE_CMDLINE into MPI_LINK_CMDLINE, if the underlying
    if (NOT MPI_LINK_CMDLINE)
      SET(MPI_LINK_CMDLINE ${MPI_COMPILE_CMDLINE})
    endif (NOT MPI_LINK_CMDLINE)
  else (MPI_COMPILER_RETURN EQUAL 0)
    message(STATUS "Unable to determine MPI from MPI driver ${MPI_COMPILER}")
  endif (MPI_COMPILER_RETURN EQUAL 0)
endif (MPI_INCLUDE_PATH AND MPI_LIBRARY)

if (MPI_INCLUDE_PATH AND MPI_LIBRARY)
  # Do nothing: we already have MPI_INCLUDE_PATH and MPI_LIBRARY in
  # the cache, and we don't want to override those settings.
elseif (MPI_COMPILE_CMDLINE)
  # Extract compile flags from the compile command line.
  string(REGEX MATCHALL "-D([^\" ]+|\"[^\"]+\")" MPI_ALL_COMPILE_FLAGS "${MPI_COMPILE_CMDLINE}")
  set(MPI_COMPILE_FLAGS_WORK)
  foreach(FLAG ${MPI_ALL_COMPILE_FLAGS})
    if (MPI_COMPILE_FLAGS_WORK)
      set(MPI_COMPILE_FLAGS_WORK "${MPI_COMPILE_FLAGS_WORK} ${FLAG}")
    else(MPI_COMPILE_FLAGS_WORK)
      set(MPI_COMPILE_FLAGS_WORK ${FLAG})
    endif(MPI_COMPILE_FLAGS_WORK)
  endforeach(FLAG)

  # Extract include paths from compile command line
  string(REGEX MATCHALL "-I([^\" ]+|\"[^\"]+\")" MPI_ALL_INCLUDE_PATHS "${MPI_COMPILE_CMDLINE}")
  set(MPI_INCLUDE_PATH_WORK)
  foreach(IPATH ${MPI_ALL_INCLUDE_PATHS})
    string(REGEX REPLACE "^-I" "" IPATH ${IPATH})
    string(REGEX REPLACE "//" "/" IPATH ${IPATH})
    list(APPEND MPI_INCLUDE_PATH_WORK ${IPATH})
  endforeach(IPATH)
  
  if (NOT MPI_INCLUDE_PATH_WORK)
    if (MPI_COMPILER_MAY_HAVE_INCLIBDIRS)
      # The compile command line didn't have any include paths on it,
      # but we may have -showme:incdirs. Use it.
      exec_program(${MPI_COMPILER} 
        ARGS -showme:incdirs
        OUTPUT_VARIABLE MPI_INCLUDE_PATH_WORK
        RETURN_VALUE MPI_COMPILER_RETURN)
      separate_arguments(MPI_INCLUDE_PATH_WORK)
    endif (MPI_COMPILER_MAY_HAVE_INCLIBDIRS)
  endif (NOT MPI_INCLUDE_PATH_WORK)

  if (NOT MPI_INCLUDE_PATH_WORK)
    # If all else fails, just search for mpi.h in the normal include
    # paths.
    find_path(MPI_INCLUDE_PATH mpi.h)
    set(MPI_INCLUDE_PATH_WORK ${MPI_INCLUDE_PATH})
  endif (NOT MPI_INCLUDE_PATH_WORK)

  # Extract linker paths from the link command line
  string(REGEX MATCHALL "-L([^\" ]+|\"[^\"]+\")" MPI_ALL_LINK_PATHS "${MPI_LINK_CMDLINE}")
  set(MPI_LINK_PATH)
  foreach(LPATH ${MPI_ALL_LINK_PATHS})
    string(REGEX REPLACE "^-L" "" LPATH ${LPATH})
    string(REGEX REPLACE "//" "/" LPATH ${LPATH})
    list(APPEND MPI_LINK_PATH ${LPATH})
  endforeach(LPATH)

  if (NOT MPI_LINK_PATH)
    if (MPI_COMPILER_MAY_HAVE_INCLIBDIRS)
      # The compile command line didn't have any linking paths on it,
      # but we may have -showme:libdirs. Use it.
      exec_program(${MPI_COMPILER} 
        ARGS -showme:libdirs
        OUTPUT_VARIABLE MPI_LINK_PATH
        RETURN_VALUE MPI_COMPILER_RETURN)
      separate_arguments(MPI_LINK_PATH)
    endif (MPI_COMPILER_MAY_HAVE_INCLIBDIRS)
  endif (NOT MPI_LINK_PATH)

  # Extract linker flags from the link command line
  string(REGEX MATCHALL "-Wl,([^\" ]+|\"[^\"]+\")" MPI_ALL_LINK_FLAGS "${MPI_LINK_CMDLINE}")
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
  string(REGEX MATCHALL "-l([^\" ]+|\"[^\"]+\")" MPI_LIBNAMES "${MPI_LINK_CMDLINE}")

  # Determine full path names for all of the libraries that one needs
  # to link against in an MPI program
  set(MPI_LIBRARIES)
  foreach(LIB ${MPI_LIBNAMES})
    string(REGEX REPLACE "^-l" "" LIB ${LIB})
    set(MPI_LIB "MPI_LIB-NOTFOUND" CACHE FILEPATH "Cleared" FORCE)
    find_library(MPI_LIB ${LIB} HINTS ${MPI_LINK_PATH})
    if (MPI_LIB)
      list(APPEND MPI_LIBRARIES ${MPI_LIB})
    else (MPI_LIB)
      message(SEND_ERROR "Unable to find MPI library ${LIB}")
    endif (MPI_LIB)
  endforeach(LIB)
  set(MPI_LIB "MPI_LIB-NOTFOUND" CACHE INTERNAL "Scratch variable for MPI detection" FORCE)

  # Chop MPI_LIBRARIES into the old-style MPI_LIBRARY and
  # MPI_EXTRA_LIBRARY.
  list(LENGTH MPI_LIBRARIES MPI_NUMLIBS)
  list(LENGTH MPI_LIBNAMES MPI_NUMLIBS_EXPECTED)
  if (MPI_NUMLIBS EQUAL MPI_NUMLIBS_EXPECTED)
    list(GET MPI_LIBRARIES 0 MPI_LIBRARY_WORK)
    set(MPI_LIBRARY ${MPI_LIBRARY_WORK} CACHE FILEPATH "MPI library to link against" FORCE)
  else (MPI_NUMLIBS EQUAL MPI_NUMLIBS_EXPECTED)
    set(MPI_LIBRARY "MPI_LIBRARY-NOTFOUND" CACHE FILEPATH "MPI library to link against" FORCE)
  endif (MPI_NUMLIBS EQUAL MPI_NUMLIBS_EXPECTED)
  if (MPI_NUMLIBS GREATER 1)
    set(MPI_EXTRA_LIBRARY_WORK ${MPI_LIBRARIES})
    list(REMOVE_AT MPI_EXTRA_LIBRARY_WORK 0)
    set(MPI_EXTRA_LIBRARY ${MPI_EXTRA_LIBRARY_WORK} CACHE STRING "Extra MPI libraries to link against" FORCE)
  else (MPI_NUMLIBS GREATER 1)
    set(MPI_EXTRA_LIBRARY "MPI_EXTRA_LIBRARY-NOTFOUND" CACHE STRING "Extra MPI libraries to link against" FORCE)
  endif (MPI_NUMLIBS GREATER 1)

  # Set up all of the appropriate cache entries
  set(MPI_COMPILE_FLAGS ${MPI_COMPILE_FLAGS_WORK} CACHE STRING "MPI compilation flags" FORCE)
  set(MPI_INCLUDE_PATH ${MPI_INCLUDE_PATH_WORK} CACHE STRING "MPI include path" FORCE)
  set(MPI_LINK_FLAGS ${MPI_LINK_FLAGS_WORK} CACHE STRING "MPI linking flags" FORCE)
else (MPI_COMPILE_CMDLINE)
  find_path(MPI_INCLUDE_PATH mpi.h 
    /usr/local/include 
    /usr/include 
    /usr/include/mpi
    /usr/local/mpi/include
    "C:/Program Files/MPICH/SDK/Include" 
    "$ENV{SystemDrive}/Program Files/MPICH2/include"
    "$ENV{SystemDrive}/Program Files/Microsoft Compute Cluster Pack/Include"
    )
  
  # Decide between 32-bit and 64-bit libraries for Microsoft's MPI
  if (CMAKE_CL_64)
    set(MS_MPI_ARCH_DIR amd64)
  else (CMAKE_CL_64)
    set(MS_MPI_ARCH_DIR i386)
  endif (CMAKE_CL_64)
  
  find_library(MPI_LIBRARY 
    NAMES mpi mpich msmpi
    PATHS /usr/lib /usr/local/lib /usr/local/mpi/lib
    "C:/Program Files/MPICH/SDK/Lib" 
    "$ENV{SystemDrive}/Program Files/MPICH/SDK/Lib"
    "$ENV{SystemDrive}/Program Files/Microsoft Compute Cluster Pack/Lib/${MS_MPI_ARCH_DIR}"
    )
  find_library(MPI_LIBRARY 
    NAMES mpich2
    PATHS
    "$ENV{SystemDrive}/Program Files/MPICH2/Lib")

  find_library(MPI_EXTRA_LIBRARY 
    NAMES mpi++
    PATHS /usr/lib /usr/local/lib /usr/local/mpi/lib
    "C:/Program Files/MPICH/SDK/Lib" 
    DOC "Extra MPI libraries to link against.")

  set(MPI_COMPILE_FLAGS "" CACHE STRING "MPI compilation flags")
  set(MPI_LINK_FLAGS "" CACHE STRING "MPI linking flags")
endif (MPI_INCLUDE_PATH AND MPI_LIBRARY)

# on BlueGene/L the MPI lib is named libmpich.rts.a, there also these additional libs are required
if("${MPI_LIBRARY}" MATCHES "mpich.rts")
   set(MPI_EXTRA_LIBRARY ${MPI_EXTRA_LIBRARY} msglayer.rts devices.rts rts.rts devices.rts)
   set(MPI_LIBRARY ${MPI_LIBRARY}  msglayer.rts devices.rts rts.rts devices.rts)
endif("${MPI_LIBRARY}" MATCHES "mpich.rts")

# Set up extra variables to conform to 
if (MPI_EXTRA_LIBRARY)
  set(MPI_LIBRARIES ${MPI_LIBRARY} ${MPI_EXTRA_LIBRARY})
else (MPI_EXTRA_LIBRARY)
  set(MPI_LIBRARIES ${MPI_LIBRARY})
endif (MPI_EXTRA_LIBRARY)

if (MPI_INCLUDE_PATH AND MPI_LIBRARY)
  set(MPI_FOUND TRUE)
else (MPI_INCLUDE_PATH AND MPI_LIBRARY)
  set(MPI_FOUND FALSE)
endif (MPI_INCLUDE_PATH AND MPI_LIBRARY)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments 
find_package_handle_standard_args(MPI DEFAULT_MSG MPI_LIBRARY MPI_INCLUDE_PATH)

mark_as_advanced(MPI_INCLUDE_PATH MPI_COMPILE_FLAGS MPI_LINK_FLAGS MPI_LIBRARY 
  MPI_EXTRA_LIBRARY)
