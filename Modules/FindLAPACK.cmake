# - Find LAPACK library
# This module finds an installed fortran library that implements the LAPACK
# linear-algebra interface (see http://www.netlib.org/lapack/).
#
# The approach follows that taken for the autoconf macro file, acx_lapack.m4
# (distributed at http://ac-archive.sourceforge.net/ac-archive/acx_lapack.html).
#
# This module sets the following variables:
#  LAPACK_FOUND - set to true if a library implementing the LAPACK interface
#    is found
#  LAPACK_LINKER_FLAGS - uncached list of required linker flags (excluding -l
#    and -L).
#  LAPACK_LIBRARIES - uncached list of libraries (using full path name) to 
#    link against to use LAPACK
#  LAPACK95_LIBRARIES - uncached list of libraries (using full path name) to 
#    link against to use LAPACK95

#
 
include(CheckFortranFunctionExists)
set(LAPACK_FOUND FALSE)

macro(Check_Lapack_Libraries LIBRARIES _prefix _name _flags _list _blas)
# This macro checks for the existence of the combination of fortran libraries
# given by _list.  If the combination is found, this macro checks (using the 
# Check_Fortran_Function_Exists macro) whether can link against that library
# combination using the name of a routine given by _name using the linker
# flags given by _flags.  If the combination of libraries is found and passes
# the link test, LIBRARIES is set to the list of complete library paths that
# have been found.  Otherwise, LIBRARIES is set to FALSE.
 
# N.B. _prefix is the prefix applied to the names of all cached variables that
# are generated internally and marked advanced by this macro.

set(_libraries_work TRUE)
set(${LIBRARIES})
set(_combined_name)
foreach(_library ${_list})
  set(_combined_name ${_combined_name}_${_library})

  if(_libraries_work)
IF (WIN32)
    find_library(${_prefix}_${_library}_LIBRARY
    NAMES ${_library}
    PATHS ENV LIB 
    )
ENDIF (WIN32)

  if(APPLE)
    find_library(${_prefix}_${_library}_LIBRARY
    NAMES ${_library}
    PATHS /usr/local/lib /usr/lib /usr/local/lib64 /usr/lib64 ENV DYLD_LIBRARY_PATH
    )
    else(APPLE)
        find_library(${_prefix}_${_library}_LIBRARY
    NAMES ${_library}
    PATHS /usr/local/lib /usr/lib /usr/local/lib64 /usr/lib64 ENV LD_LIBRARY_PATH
    )
    endif(APPLE)

    mark_as_advanced(${_prefix}_${_library}_LIBRARY)
    set(${LIBRARIES} ${${LIBRARIES}} ${${_prefix}_${_library}_LIBRARY})
    set(_libraries_work ${${_prefix}_${_library}_LIBRARY})
  endif(_libraries_work)
endforeach(_library ${_list})

if(_libraries_work)
  # Test this combination of libraries.
  set(CMAKE_REQUIRED_LIBRARIES ${_flags} ${${LIBRARIES}} ${_blas})
  #message("DEBUG: CMAKE_REQUIRED_LIBRARIES = ${CMAKE_REQUIRED_LIBRARIES}")
  check_fortran_function_exists(${_name} ${_prefix}${_combined_name}_WORKS)
  set(CMAKE_REQUIRED_LIBRARIES)
  mark_as_advanced(${_prefix}${_combined_name}_WORKS)
  set(_libraries_work ${${_prefix}${_combined_name}_WORKS})
  #message("DEBUG: ${LIBRARIES} = ${${LIBRARIES}}")
endif(_libraries_work)

if(NOT _libraries_work)
  set(${LIBRARIES} FALSE)
endif(NOT _libraries_work)

endmacro(Check_Lapack_Libraries)


set(LAPACK_LINKER_FLAGS)
set(LAPACK_LIBRARIES)
set(LAPACK95_LIBRARIES)


if(LAPACK_FIND_QUIETLY OR NOT LAPACK_FIND_REQUIRED)
  find_package(BLAS)
else(LAPACK_FIND_QUIETLY OR NOT LAPACK_FIND_REQUIRED)
  find_package(BLAS REQUIRED)
endif(LAPACK_FIND_QUIETLY OR NOT LAPACK_FIND_REQUIRED)

if(BLAS_FOUND)
  set(LAPACK_LINKER_FLAGS ${BLAS_LINKER_FLAGS})

#intel lapack
  if(NOT LAPACK_LIBRARIES)

  check_lapack_libraries(
  LAPACK_LIBRARIES
  LAPACK
  cheev
  ""
  "mkl_lapack"
 "${BLAS_LIBRARIES}"
  )
  endif(NOT LAPACK_LIBRARIES)

  if(NOT LAPACK95_LIBRARIES)
  check_lapack_libraries(
  LAPACK95_LIBRARIES
  LAPACK
  cheev
  ""
  "mkl_lapack95"
 "${BLAS_LIBRARIES}"
  )
  endif(NOT LAPACK95_LIBRARIES)

#acml lapack
  if(NOT LAPACK_LIBRARIES)

  check_lapack_libraries(
  LAPACK_LIBRARIES
  LAPACK
  cheev
  ""
  "acml"
 "${BLAS_LIBRARIES}"
  )
endif(NOT LAPACK_LIBRARIES)


# Apple LAPACK library?
if(NOT LAPACK_LIBRARIES)
  check_lapack_libraries(
  LAPACK_LIBRARIES
  LAPACK
  cheev
  ""
  "Accelerate"
  "${BLAS_LIBRARIES}"
  )
  endif(NOT LAPACK_LIBRARIES)
  
  if ( NOT LAPACK_LIBRARIES )
    check_lapack_libraries(
    LAPACK_LIBRARIES
    LAPACK
    cheev
    ""
    "vecLib"
    "${BLAS_LIBRARIES}"
    )
  endif ( NOT LAPACK_LIBRARIES )

# Generic LAPACK library?
  if ( NOT LAPACK_LIBRARIES )
    check_lapack_libraries(
    LAPACK_LIBRARIES
    LAPACK
    cheev
    ""
    "lapack"
    "${BLAS_LIBRARIES}"
    )
  endif ( NOT LAPACK_LIBRARIES )

else(BLAS_FOUND)
  message(STATUS "LAPACK requires BLAS")
endif(BLAS_FOUND)

if(LAPACK_LIBRARIES)
  set(LAPACK_FOUND TRUE)
else(LAPACK_LIBRARIES)
  set(LAPACK_FOUND FALSE)
endif(LAPACK_LIBRARIES)

if(NOT LAPACK_FIND_QUIETLY)
  if(LAPACK_FOUND)
    message(STATUS "A library with LAPACK API found.")
  else(LAPACK_FOUND)
    if(LAPACK_FIND_REQUIRED)
      message(FATAL_ERROR 
      "A required library with LAPACK API not found. Please specify library location."
      )
    else(LAPACK_FIND_REQUIRED)
      message(STATUS
      "A library with LAPACK API not found. Please specify library location."
      )
    endif(LAPACK_FIND_REQUIRED)
  endif(LAPACK_FOUND)
endif(NOT LAPACK_FIND_QUIETLY)
