# - Find BLAS library
# This module finds an installed fortran library that implements the BLAS 
# linear-algebra interface (see http://www.netlib.org/blas/).  
# The list of libraries searched for is taken
# from the autoconf macro file, acx_blas.m4 (distributed at
# http://ac-archive.sourceforge.net/ac-archive/acx_blas.html).
#
# This module sets the following variables:
#  BLAS_FOUND - set to true if a library implementing the BLAS interface
#    is found
#  BLAS_LINKER_FLAGS - uncached list of required linker flags (excluding -l
#    and -L).
#  BLAS_LIBRARIES - uncached list of libraries (using full path name) to 
#    link against to use BLAS
#  BLAS95_LIBRARIES - uncached list of libraries (using full path name) # to link against to use BLAS95 interface
#

include(CheckFortranFunctionExists)

macro(Check_Fortran_Libraries LIBRARIES _prefix _name _flags _list)
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

   if ( WIN32 )
    find_library(${_prefix}_${_library}_LIBRARY
    NAMES ${_library}
    PATHS ENV LIB 
    )
   endif ( WIN32 )
    
   if ( APPLE ) 
    find_library(${_prefix}_${_library}_LIBRARY
    NAMES ${_library}
    PATHS /usr/local/lib /usr/lib /usr/local/lib64 /usr/lib64 ENV DYLD_LIBRARY_PATH 
    )
   
   else ( APPLE )
    find_library(${_prefix}_${_library}_LIBRARY
    NAMES ${_library}
    PATHS /usr/local/lib /usr/lib /usr/local/lib64 /usr/lib64 ENV LD_LIBRARY_PATH 
    )
   endif( APPLE )
    mark_as_advanced(${_prefix}_${_library}_LIBRARY)
    set(${LIBRARIES} ${${LIBRARIES}} ${${_prefix}_${_library}_LIBRARY})
    set(_libraries_work ${${_prefix}_${_library}_LIBRARY})
  endif(_libraries_work)
endforeach(_library ${_list})
if(_libraries_work)
  # Test this combination of libraries.
  set(CMAKE_REQUIRED_LIBRARIES ${_flags} ${${LIBRARIES}})
  #message("DEBUG: CMAKE_REQUIRED_LIBRARIES = ${CMAKE_REQUIRED_LIBRARIES}")
  check_fortran_function_exists(${_name} ${_prefix}${_combined_name}_WORKS)
  set(CMAKE_REQUIRED_LIBRARIES)
  mark_as_advanced(${_prefix}${_combined_name}_WORKS)
  set(_libraries_work ${${_prefix}${_combined_name}_WORKS})
endif(_libraries_work)
if(NOT _libraries_work)
  set(${LIBRARIES} FALSE)
endif(NOT _libraries_work)
#message("DEBUG: ${LIBRARIES} = ${${LIBRARIES}}")
endmacro(Check_Fortran_Libraries)

set(BLAS_LINKER_FLAGS)
set(BLAS_LIBRARIES)
set(BLAS95_LIBRARIES)



if(NOT BLAS_LIBRARIES)
  # BLAS in ATLAS library? (http://math-atlas.sourceforge.net/)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  cblas_dgemm
  ""
  "cblas;f77blas;atlas"
  )
endif(NOT BLAS_LIBRARIES)

# BLAS in PhiPACK libraries? (requires generic BLAS lib, too)
if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  ""
  "sgemm;dgemm;blas"
  )
endif(NOT BLAS_LIBRARIES)

# BLAS in Alpha CXML library?
if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  ""
  "cxml"
  )
endif(NOT BLAS_LIBRARIES)

# BLAS in Alpha DXML library? (now called CXML, see above)
if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  ""
  "dxml"
  )
endif(NOT BLAS_LIBRARIES)

# BLAS in Sun Performance library?
if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  "-xlic_lib=sunperf"
  "sunperf;sunmath"
  )
  if(BLAS_LIBRARIES)
    set(BLAS_LINKER_FLAGS "-xlic_lib=sunperf")
  endif(BLAS_LIBRARIES)
  
endif(NOT BLAS_LIBRARIES)

# BLAS in SCSL library?  (SGI/Cray Scientific Library)
if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  ""
  "scsl"
  )
endif(NOT BLAS_LIBRARIES)

# BLAS in SGIMATH library?
if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  ""
  "complib.sgimath"
  )
endif(NOT BLAS_LIBRARIES)

# BLAS in IBM ESSL library? (requires generic BLAS lib, too)
if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  ""
  "essl;blas"
  )
endif(NOT BLAS_LIBRARIES)




#BLAS in intel mkl 10 library? (em64t 64bit)
if(NOT BLAS_LIBRARIES)
check_fortran_libraries(
BLAS_LIBRARIES
BLAS
sgemm
""
"mkl_intel_lp64;mkl_intel_thread;mkl_core;guide;pthread"
)
endif(NOT BLAS_LIBRARIES)
if(NOT BLAS95_LIBRARIES)
check_fortran_libraries(
BLAS95_LIBRARIES
BLAS
sgemm
""
"mkl_blas95;mkl_intel_lp64;mkl_intel_thread;mkl_core;guide;pthread"
)
endif(NOT BLAS95_LIBRARIES)

### windows version of intel mkl 10

if(NOT BLAS_LIBRARIES)
check_fortran_libraries(
BLAS_LIBRARIES
BLAS
SGEMM
""
"mkl_c_dll;mkl_intel_thread_dll;mkl_core_dll;libguide40"
)
endif(NOT BLAS_LIBRARIES)

if(NOT BLAS95_LIBRARIES)
check_fortran_libraries(
BLAS95_LIBRARIES
BLAS
sgemm
""
"mkl_blas95;mkl_intel_c;mkl_intel_thread;mkl_core;libguide40"
)
endif(NOT BLAS95_LIBRARIES)


# linux 32 bit
if(NOT BLAS95_LIBRARIES)
check_fortran_libraries(
BLAS95_LIBRARIES
BLAS
sgemm
""
"mkl_blas95;mkl_intel;mkl_intel_thread;mkl_core;guide;pthread"
)
endif(NOT BLAS95_LIBRARIES)


#older vesions of intel mkl libs

# BLAS in intel mkl library? (shared)
if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  ""
  "mkl;guide;pthread"
  )
endif(NOT BLAS_LIBRARIES)


#BLAS in intel mkl library? (static, 32bit)
if(NOT BLAS_LIBRARIES)
check_fortran_libraries(
BLAS_LIBRARIES
BLAS
sgemm
""
"mkl_ia32;guide;pthread"
)
endif(NOT BLAS_LIBRARIES)

#BLAS in intel mkl library? (static, em64t 64bit)
if(NOT BLAS_LIBRARIES)
check_fortran_libraries(
BLAS_LIBRARIES
BLAS
sgemm
""
"mkl_em64t;guide;pthread"
)
endif(NOT BLAS_LIBRARIES)


#BLAS in acml library? 
if(NOT BLAS_LIBRARIES)
check_fortran_libraries(
BLAS_LIBRARIES
BLAS
sgemm
""
"acml"
)
endif(NOT BLAS_LIBRARIES)

# Apple BLAS library?
if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  cblas_dgemm
  ""
  "Accelerate"
  )
  
  endif(NOT BLAS_LIBRARIES)
  
  if ( NOT BLAS_LIBRARIES )
    check_fortran_libraries(
    BLAS_LIBRARIES
    BLAS
    cblas_dgemm
    ""
    "vecLib"
    )
  endif ( NOT BLAS_LIBRARIES )


# Generic BLAS library?
if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  ""
  "blas"
  )
endif(NOT BLAS_LIBRARIES)


if(BLAS_LIBRARIES)
  set(BLAS_FOUND TRUE)
else(BLAS_LIBRARIES)
  set(BLAS_FOUND FALSE)
endif(BLAS_LIBRARIES)

if(NOT BLAS_FIND_QUIETLY)
  if(BLAS_FOUND)
    message(STATUS "A library with BLAS API found.")
  else(BLAS_FOUND)
    if(BLAS_FIND_REQUIRED)
      message(FATAL_ERROR 
      "A required library with BLAS API not found. Please specify library location."
      )
    else(BLAS_FIND_REQUIRED)
      message(STATUS
      "A library with BLAS API not found. Please specify library location."
      )
    endif(BLAS_FIND_REQUIRED)
  endif(BLAS_FOUND)
endif(NOT BLAS_FIND_QUIETLY)
