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
#  BLAS95_LIBRARIES - uncached list of libraries (using full path name)
#    to link against to use BLAS95 interface
#  BLAS95_FOUND - set to true if a library implementing the BLAS f95 interface
#    is found
#  BLA_STATIC  if set on this determines what kind of linkage we do (static)
#  BLA_VENDOR  if set checks only the specified vendor, if not set checks
#     all the possibilities
#  BLA_F95     if set on tries to find the f95 interfaces for BLAS/LAPACK
##########
### List of vendors (BLA_VENDOR) valid in this module
##  Goto,ATLAS PhiPACK,CXML,DXML,SunPerf,SCSL,SGIMATH,IBMESSL,Intel10_32 (intel mkl v10 32 bit),Intel10_64lp (intel mkl v10 64 bit,lp thread model, lp64 model),
##  Intel( older versions of mkl 32 and 64 bit), ACML,ACML_MP,ACML_GPU,Apple, NAS, Generic
# C/CXX should be enabled to use Intel mkl

#=============================================================================
# Copyright 2007-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

include(CheckFunctionExists)
include(CheckFortranFunctionExists)

set(_blas_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})

# Check the language being used
get_property( _LANGUAGES_ GLOBAL PROPERTY ENABLED_LANGUAGES )
if( _LANGUAGES_ MATCHES Fortran )
  set( _CHECK_FORTRAN TRUE )
elseif( (_LANGUAGES_ MATCHES C) OR (_LANGUAGES_ MATCHES CXX) )
  set( _CHECK_FORTRAN FALSE )
else()
  if(BLAS_FIND_REQUIRED)
    message(FATAL_ERROR "FindBLAS requires Fortran, C, or C++ to be enabled.")
  else(BLAS_FIND_REQUIRED)
    message(STATUS "Looking for BLAS... - NOT found (Unsupported languages)")
    return()
  endif(BLAS_FIND_REQUIRED)
endif( )

macro(Check_Fortran_Libraries LIBRARIES _prefix _name _flags _list _thread)
# This macro checks for the existence of the combination of fortran libraries
# given by _list.  If the combination is found, this macro checks (using the
# Check_Fortran_Function_Exists macro) whether can link against that library
# combination using the name of a routine given by _name using the linker
# flags given by _flags.  If the combination of libraries is found and passes
# the link test, LIBRARIES is set to the list of complete library paths that
# have been found.  Otherwise, LIBRARIES is set to FALSE.

# N.B. _prefix is the prefix applied to the names of all cached variables that
# are generated internally and marked advanced by this macro.

set(_libdir ${ARGN})

set(_libraries_work TRUE)
set(${LIBRARIES})
set(_combined_name)
if (NOT _libdir)
  if (WIN32)
    set(_libdir ENV LIB)
  elseif (APPLE)
    set(_libdir /usr/local/lib /usr/lib /usr/local/lib64 /usr/lib64 ENV DYLD_LIBRARY_PATH)
  else ()
    set(_libdir /usr/local/lib /usr/lib /usr/local/lib64 /usr/lib64 ENV LD_LIBRARY_PATH)
  endif ()
endif ()
foreach(_library ${_list})
  set(_combined_name ${_combined_name}_${_library})

  if(_libraries_work)
    if (BLA_STATIC)
      if (WIN32)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .lib ${CMAKE_FIND_LIBRARY_SUFFIXES})
      endif ( WIN32 )
      if (APPLE)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .lib ${CMAKE_FIND_LIBRARY_SUFFIXES})
      else (APPLE)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
      endif (APPLE)
    else (BLA_STATIC)
      if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        # for ubuntu's libblas3gf and liblapack3gf packages
        set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES} .so.3gf)
      endif ()
    endif (BLA_STATIC)
    find_library(${_prefix}_${_library}_LIBRARY
      NAMES ${_library}
      PATHS ${_libdir}
      )
    mark_as_advanced(${_prefix}_${_library}_LIBRARY)
    set(${LIBRARIES} ${${LIBRARIES}} ${${_prefix}_${_library}_LIBRARY})
    set(_libraries_work ${${_prefix}_${_library}_LIBRARY})
  endif(_libraries_work)
endforeach(_library ${_list})
if(_libraries_work)
  # Test this combination of libraries.
  set(CMAKE_REQUIRED_LIBRARIES ${_flags} ${${LIBRARIES}} ${_threads})
#  message("DEBUG: CMAKE_REQUIRED_LIBRARIES = ${CMAKE_REQUIRED_LIBRARIES}")
  if (_CHECK_FORTRAN)
    check_fortran_function_exists("${_name}" ${_prefix}${_combined_name}_WORKS)
  else()
    check_function_exists("${_name}_" ${_prefix}${_combined_name}_WORKS)
  endif()
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
if ($ENV{BLA_VENDOR} MATCHES ".+")
  set(BLA_VENDOR $ENV{BLA_VENDOR})
else ($ENV{BLA_VENDOR} MATCHES ".+")
  if(NOT BLA_VENDOR)
    set(BLA_VENDOR "All")
  endif(NOT BLA_VENDOR)
endif ($ENV{BLA_VENDOR} MATCHES ".+")

if (BLA_VENDOR STREQUAL "Goto" OR BLA_VENDOR STREQUAL "All")
 if(NOT BLAS_LIBRARIES)
  # gotoblas (http://www.tacc.utexas.edu/tacc-projects/gotoblas2)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  ""
  "goto2"
  ""
  )
 endif(NOT BLAS_LIBRARIES)
endif (BLA_VENDOR STREQUAL "Goto" OR BLA_VENDOR STREQUAL "All")

if (BLA_VENDOR STREQUAL "ATLAS" OR BLA_VENDOR STREQUAL "All")
 if(NOT BLAS_LIBRARIES)
  # BLAS in ATLAS library? (http://math-atlas.sourceforge.net/)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  dgemm
  ""
  "f77blas;atlas"
  ""
  )
 endif(NOT BLAS_LIBRARIES)
endif (BLA_VENDOR STREQUAL "ATLAS" OR BLA_VENDOR STREQUAL "All")

# BLAS in PhiPACK libraries? (requires generic BLAS lib, too)
if (BLA_VENDOR STREQUAL "PhiPACK" OR BLA_VENDOR STREQUAL "All")
 if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  ""
  "sgemm;dgemm;blas"
  ""
  )
 endif(NOT BLAS_LIBRARIES)
endif (BLA_VENDOR STREQUAL "PhiPACK" OR BLA_VENDOR STREQUAL "All")

# BLAS in Alpha CXML library?
if (BLA_VENDOR STREQUAL "CXML" OR BLA_VENDOR STREQUAL "All")
 if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  ""
  "cxml"
  ""
  )
 endif(NOT BLAS_LIBRARIES)
endif (BLA_VENDOR STREQUAL "CXML" OR BLA_VENDOR STREQUAL "All")

# BLAS in Alpha DXML library? (now called CXML, see above)
if (BLA_VENDOR STREQUAL "DXML" OR BLA_VENDOR STREQUAL "All")
 if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  ""
  "dxml"
  ""
  )
 endif(NOT BLAS_LIBRARIES)
endif (BLA_VENDOR STREQUAL "DXML" OR BLA_VENDOR STREQUAL "All")

# BLAS in Sun Performance library?
if (BLA_VENDOR STREQUAL "SunPerf" OR BLA_VENDOR STREQUAL "All")
 if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  "-xlic_lib=sunperf"
  "sunperf;sunmath"
  ""
  )
  if(BLAS_LIBRARIES)
    set(BLAS_LINKER_FLAGS "-xlic_lib=sunperf")
  endif(BLAS_LIBRARIES)
 endif(NOT BLAS_LIBRARIES)
endif (BLA_VENDOR STREQUAL "SunPerf" OR BLA_VENDOR STREQUAL "All")

# BLAS in SCSL library?  (SGI/Cray Scientific Library)
if (BLA_VENDOR STREQUAL "SCSL" OR BLA_VENDOR STREQUAL "All")
 if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  ""
  "scsl"
  ""
  )
 endif(NOT BLAS_LIBRARIES)
endif (BLA_VENDOR STREQUAL "SCSL" OR BLA_VENDOR STREQUAL "All")

# BLAS in SGIMATH library?
if (BLA_VENDOR STREQUAL "SGIMATH" OR BLA_VENDOR STREQUAL "All")
 if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  ""
  "complib.sgimath"
  ""
  )
 endif(NOT BLAS_LIBRARIES)
endif (BLA_VENDOR STREQUAL "SGIMATH" OR BLA_VENDOR STREQUAL "All")

# BLAS in IBM ESSL library? (requires generic BLAS lib, too)
if (BLA_VENDOR STREQUAL "IBMESSL" OR BLA_VENDOR STREQUAL "All")
 if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  ""
  "essl;blas"
  ""
  )
 endif(NOT BLAS_LIBRARIES)
endif (BLA_VENDOR STREQUAL "IBMESSL" OR BLA_VENDOR STREQUAL "All")

#BLAS in acml library?
if (BLA_VENDOR MATCHES "ACML.*" OR BLA_VENDOR STREQUAL "All")
 if( ((BLA_VENDOR STREQUAL "ACML") AND (NOT BLAS_ACML_LIB_DIRS)) OR
     ((BLA_VENDOR STREQUAL "ACML_MP") AND (NOT BLAS_ACML_MP_LIB_DIRS)) OR
     ((BLA_VENDOR STREQUAL "ACML_GPU") AND (NOT BLAS_ACML_GPU_LIB_DIRS))
   )
   # try to find acml in "standard" paths
   if( WIN32 )
    file( GLOB _ACML_ROOT "C:/AMD/acml*/ACML-EULA.txt" )
   else()
    file( GLOB _ACML_ROOT "/opt/acml*/ACML-EULA.txt" )
   endif()
   if( WIN32 )
    file( GLOB _ACML_GPU_ROOT "C:/AMD/acml*/GPGPUexamples" )
   else()
    file( GLOB _ACML_GPU_ROOT "/opt/acml*/GPGPUexamples" )
   endif()
   list(GET _ACML_ROOT 0 _ACML_ROOT)
   list(GET _ACML_GPU_ROOT 0 _ACML_GPU_ROOT)
   if( _ACML_ROOT )
    get_filename_component( _ACML_ROOT ${_ACML_ROOT} PATH )
    if( SIZEOF_INTEGER EQUAL 8 )
     set( _ACML_PATH_SUFFIX "_int64" )
    else()
    set( _ACML_PATH_SUFFIX "" )
   endif()
   if( CMAKE_Fortran_COMPILER_ID STREQUAL "Intel" )
    set( _ACML_COMPILER32 "ifort32" )
    set( _ACML_COMPILER64 "ifort64" )
   elseif( CMAKE_Fortran_COMPILER_ID STREQUAL "SunPro" )
    set( _ACML_COMPILER32 "sun32" )
    set( _ACML_COMPILER64 "sun64" )
   elseif( CMAKE_Fortran_COMPILER_ID STREQUAL "PGI" )
    set( _ACML_COMPILER32 "pgi32" )
    if( WIN32 )
     set( _ACML_COMPILER64 "win64" )
    else()
     set( _ACML_COMPILER64 "pgi64" )
    endif()
   elseif( CMAKE_Fortran_COMPILER_ID STREQUAL "Open64" )
    # 32 bit builds not supported on Open64 but for code simplicity
    # We'll just use the same directory twice
    set( _ACML_COMPILER32 "open64_64" )
    set( _ACML_COMPILER64 "open64_64" )
   elseif( CMAKE_Fortran_COMPILER_ID STREQUAL "NAG" )
    set( _ACML_COMPILER32 "nag32" )
    set( _ACML_COMPILER64 "nag64" )
   else() #if( CMAKE_Fortran_COMPILER_ID STREQUAL "GNU" )
    set( _ACML_COMPILER32 "gfortran32" )
    set( _ACML_COMPILER64 "gfortran64" )
   endif()

   if( BLA_VENDOR STREQUAL "ACML_MP" )
    set(_ACML_MP_LIB_DIRS
     "${_ACML_ROOT}/${_ACML_COMPILER32}_mp${_ACML_PATH_SUFFIX}/lib"
     "${_ACML_ROOT}/${_ACML_COMPILER64}_mp${_ACML_PATH_SUFFIX}/lib" )
   else() #if( _BLAS_VENDOR STREQUAL "ACML" )
    set(_ACML_LIB_DIRS
     "${_ACML_ROOT}/${_ACML_COMPILER32}${_ACML_PATH_SUFFIX}/lib"
     "${_ACML_ROOT}/${_ACML_COMPILER64}${_ACML_PATH_SUFFIX}/lib" )
   endif()
  endif()
 elseif(BLAS_${BLA_VENDOR}_LIB_DIRS)
   set(_${BLA_VENDOR}_LIB_DIRS ${BLAS_${BLA_VENDOR}_LIB_DIRS})
 endif()

 if( BLA_VENDOR STREQUAL "ACML_MP" )
  foreach( BLAS_ACML_MP_LIB_DIRS ${_ACML_MP_LIB_DIRS})
   check_fortran_libraries (
     BLAS_LIBRARIES
     BLAS
     sgemm
     "" "acml_mp;acml_mv" "" ${BLAS_ACML_MP_LIB_DIRS}
   )
   if( BLAS_LIBRARIES )
    break()
   endif()
  endforeach()
 elseif( BLA_VENDOR STREQUAL "ACML_GPU" )
  foreach( BLAS_ACML_GPU_LIB_DIRS ${_ACML_GPU_LIB_DIRS})
   check_fortran_libraries (
     BLAS_LIBRARIES
     BLAS
     sgemm
     "" "acml;acml_mv;CALBLAS" "" ${BLAS_ACML_GPU_LIB_DIRS}
   )
   if( BLAS_LIBRARIES )
    break()
   endif()
  endforeach()
 else() #if( _BLAS_VENDOR STREQUAL "ACML" )
  foreach( BLAS_ACML_LIB_DIRS ${_ACML_LIB_DIRS} )
   check_fortran_libraries (
     BLAS_LIBRARIES
     BLAS
     sgemm
     "" "acml;acml_mv" "" ${BLAS_ACML_LIB_DIRS}
   )
   if( BLAS_LIBRARIES )
    break()
   endif()
  endforeach()
 endif()

 # Either acml or acml_mp should be in LD_LIBRARY_PATH but not both
 if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  ""
  "acml;acml_mv"
  ""
  )
 endif(NOT BLAS_LIBRARIES)
 if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  ""
  "acml_mp;acml_mv"
  ""
  )
 endif(NOT BLAS_LIBRARIES)
 if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  ""
  "acml;acml_mv;CALBLAS"
  ""
  )
 endif(NOT BLAS_LIBRARIES)
endif () # ACML

# Apple BLAS library?
if (BLA_VENDOR STREQUAL "Apple" OR BLA_VENDOR STREQUAL "All")
if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  dgemm
  ""
  "Accelerate"
  ""
  )
 endif(NOT BLAS_LIBRARIES)
endif (BLA_VENDOR STREQUAL "Apple" OR BLA_VENDOR STREQUAL "All")

if (BLA_VENDOR STREQUAL "NAS" OR BLA_VENDOR STREQUAL "All")
 if ( NOT BLAS_LIBRARIES )
    check_fortran_libraries(
    BLAS_LIBRARIES
    BLAS
    dgemm
    ""
    "vecLib"
    ""
    )
 endif ( NOT BLAS_LIBRARIES )
endif (BLA_VENDOR STREQUAL "NAS" OR BLA_VENDOR STREQUAL "All")
# Generic BLAS library?
if (BLA_VENDOR STREQUAL "Generic" OR BLA_VENDOR STREQUAL "All")
 if(NOT BLAS_LIBRARIES)
  check_fortran_libraries(
  BLAS_LIBRARIES
  BLAS
  sgemm
  ""
  "blas"
  ""
  )
 endif(NOT BLAS_LIBRARIES)
endif (BLA_VENDOR STREQUAL "Generic" OR BLA_VENDOR STREQUAL "All")

#BLAS in intel mkl 10 library? (em64t 64bit)
if (BLA_VENDOR MATCHES "Intel*" OR BLA_VENDOR STREQUAL "All")
 if (NOT WIN32)
  set(LM "-lm")
 endif ()
 if (_LANGUAGES_ MATCHES C OR _LANGUAGES_ MATCHES CXX)
  if(BLAS_FIND_QUIETLY OR NOT BLAS_FIND_REQUIRED)
    find_package(Threads)
  else(BLAS_FIND_QUIETLY OR NOT BLAS_FIND_REQUIRED)
    find_package(Threads REQUIRED)
  endif(BLAS_FIND_QUIETLY OR NOT BLAS_FIND_REQUIRED)
  if (WIN32)
  if(BLA_F95)
    if(NOT BLAS95_LIBRARIES)
    check_fortran_libraries(
    BLAS95_LIBRARIES
    BLAS
    sgemm
    ""
    "mkl_blas95;mkl_intel_c;mkl_intel_thread;mkl_core;libguide40"
    ""
    )
    endif(NOT BLAS95_LIBRARIES)
  else(BLA_F95)
    if(NOT BLAS_LIBRARIES)
    check_fortran_libraries(
    BLAS_LIBRARIES
    BLAS
    SGEMM
    ""
    "mkl_c_dll;mkl_intel_thread_dll;mkl_core_dll;libguide40"
    ""
    )
    endif(NOT BLAS_LIBRARIES)
  endif(BLA_F95)
  else(WIN32)
  if (BLA_VENDOR STREQUAL "Intel10_32" OR BLA_VENDOR STREQUAL "All")
    if(BLA_F95)
      if(NOT BLAS95_LIBRARIES)
      check_fortran_libraries(
      BLAS95_LIBRARIES
      BLAS
      sgemm
      ""
      "mkl_blas95;mkl_intel;mkl_intel_thread;mkl_core;guide"
      "${CMAKE_THREAD_LIBS_INIT};${LM}"
      )
      endif(NOT BLAS95_LIBRARIES)
    else(BLA_F95)
    if(NOT BLAS_LIBRARIES)
      check_fortran_libraries(
      BLAS_LIBRARIES
      BLAS
      sgemm
      ""
      "mkl_intel;mkl_intel_thread;mkl_core;guide"
      "${CMAKE_THREAD_LIBS_INIT}"
      "${LM}"
      )
      endif(NOT BLAS_LIBRARIES)
    endif(BLA_F95)
  endif (BLA_VENDOR STREQUAL "Intel10_32" OR BLA_VENDOR STREQUAL "All")
  if (BLA_VENDOR STREQUAL "Intel10_64lp" OR BLA_VENDOR STREQUAL "All")
   if(BLA_F95)
    if(NOT BLAS95_LIBRARIES)
      check_fortran_libraries(
      BLAS95_LIBRARIES
      BLAS
      sgemm
      ""
      "mkl_blas95;mkl_intel_lp64;mkl_intel_thread;mkl_core;guide"
      "${CMAKE_THREAD_LIBS_INIT};${LM}"
      )
    endif(NOT BLAS95_LIBRARIES)
   else(BLA_F95)
     if(NOT BLAS_LIBRARIES)
      check_fortran_libraries(
      BLAS_LIBRARIES
      BLAS
      sgemm
      ""
      "mkl_intel_lp64;mkl_intel_thread;mkl_core;guide"
      "${CMAKE_THREAD_LIBS_INIT};${LM}"
      )
     endif(NOT BLAS_LIBRARIES)
   endif(BLA_F95)
  endif (BLA_VENDOR STREQUAL "Intel10_64lp" OR BLA_VENDOR STREQUAL "All")
  endif (WIN32)
  #older vesions of intel mkl libs
  # BLAS in intel mkl library? (shared)
  if(NOT BLAS_LIBRARIES)
    check_fortran_libraries(
    BLAS_LIBRARIES
    BLAS
    sgemm
    ""
    "mkl;guide"
    "${CMAKE_THREAD_LIBS_INIT};${LM}"
    )
  endif(NOT BLAS_LIBRARIES)
  #BLAS in intel mkl library? (static, 32bit)
  if(NOT BLAS_LIBRARIES)
    check_fortran_libraries(
    BLAS_LIBRARIES
    BLAS
    sgemm
    ""
    "mkl_ia32;guide"
    "${CMAKE_THREAD_LIBS_INIT};${LM}"
    )
  endif(NOT BLAS_LIBRARIES)
  #BLAS in intel mkl library? (static, em64t 64bit)
  if(NOT BLAS_LIBRARIES)
    check_fortran_libraries(
    BLAS_LIBRARIES
    BLAS
    sgemm
    ""
    "mkl_em64t;guide"
    "${CMAKE_THREAD_LIBS_INIT};${LM}"
    )
  endif(NOT BLAS_LIBRARIES)
 endif (_LANGUAGES_ MATCHES C OR _LANGUAGES_ MATCHES CXX)
endif (BLA_VENDOR MATCHES "Intel*" OR BLA_VENDOR STREQUAL "All")


if(BLA_F95)
 if(BLAS95_LIBRARIES)
    set(BLAS95_FOUND TRUE)
  else(BLAS95_LIBRARIES)
    set(BLAS95_FOUND FALSE)
  endif(BLAS95_LIBRARIES)

  if(NOT BLAS_FIND_QUIETLY)
    if(BLAS95_FOUND)
      message(STATUS "A library with BLAS95 API found.")
    else(BLAS95_FOUND)
      if(BLAS_FIND_REQUIRED)
        message(FATAL_ERROR
        "A required library with BLAS95 API not found. Please specify library location.")
      else(BLAS_FIND_REQUIRED)
        message(STATUS
        "A library with BLAS95 API not found. Please specify library location.")
      endif(BLAS_FIND_REQUIRED)
    endif(BLAS95_FOUND)
  endif(NOT BLAS_FIND_QUIETLY)
  set(BLAS_FOUND TRUE)
  set(BLAS_LIBRARIES "${BLAS95_LIBRARIES}")
else(BLA_F95)
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
endif(BLA_F95)

set(CMAKE_FIND_LIBRARY_SUFFIXES ${_blas_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES})
