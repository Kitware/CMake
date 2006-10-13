
# determine the compiler to use for C programs
# NOTE, a generator may set CMAKE_C_COMPILER before
# loading this file to force a compiler.
# use environment variable CCC first if defined by user, next use 
# the cmake variable CMAKE_GENERATOR_CC which can be defined by a generator
# as a default compiler
IF(NOT CMAKE_Fortran_COMPILER)
  # prefer the environment variable CC
  IF($ENV{FC} MATCHES ".+")
    GET_FILENAME_COMPONENT(CMAKE_Fortran_COMPILER_INIT $ENV{FC} PROGRAM PROGRAM_ARGS CMAKE_Fortran_FLAGS_ENV_INIT)
    IF(CMAKE_Fortran_FLAGS_ENV_INIT)
      SET(CMAKE_Fortran_COMPILER_ARG1 "${CMAKE_Fortran_FLAGS_ENV_INIT}" CACHE STRING "First argument to Fortran compiler")
    ENDIF(CMAKE_Fortran_FLAGS_ENV_INIT)
    IF(EXISTS ${CMAKE_Fortran_COMPILER_INIT})
    ELSE(EXISTS ${CMAKE_Fortran_COMPILER_INIT})
      MESSAGE(FATAL_ERROR "Could not find compiler set in environment variable FC:\n$ENV{FC}.") 
    ENDIF(EXISTS ${CMAKE_Fortran_COMPILER_INIT})
  ENDIF($ENV{FC} MATCHES ".+")
  
  # next try prefer the compiler specified by the generator
  IF(CMAKE_GENERATOR_FC) 
    IF(NOT CMAKE_Fortran_COMPILER_INIT)
      SET(CMAKE_Fortran_COMPILER_INIT ${CMAKE_GENERATOR_FC})
    ENDIF(NOT CMAKE_Fortran_COMPILER_INIT)
  ENDIF(CMAKE_GENERATOR_FC)
  
  # finally list compilers to try
  IF(CMAKE_Fortran_COMPILER_INIT)
    SET(CMAKE_Fortran_COMPILER_LIST ${CMAKE_Fortran_COMPILER_INIT})
  ELSE(CMAKE_Fortran_COMPILER_INIT)
    # Known compilers:
    #  f77/f90/f95: generic compiler names
    #  g77: GNU Fortran 77 compiler
    #  gfortran: putative GNU Fortran 95+ compiler (in progress)
    #  fort77: native F77 compiler under HP-UX (and some older Crays)
    #  frt: Fujitsu F77 compiler
    #  pgf77/pgf90/pgf95: Portland Group F77/F90/F95 compilers
    #  xlf/xlf90/xlf95: IBM (AIX) F77/F90/F95 compilers
    #  lf95: Lahey-Fujitsu F95 compiler
    #  fl32: Microsoft Fortran 77 "PowerStation" compiler
    #  af77: Apogee F77 compiler for Intergraph hardware running CLIX
    #  epcf90: "Edinburgh Portable Compiler" F90
    #  fort: Compaq (now HP) Fortran 90/95 compiler for Tru64 and Linux/Alpha
    #  ifc: Intel Fortran 95 compiler for Linux/x86
    #  efc: Intel Fortran 95 compiler for IA64
    #
    #  The order is 95 or newer compilers first, then 90, 
    #  then 77 or older compilers, gnu is always last in the group,
    #  so if you paid for a compiler it is picked by default.
    # NOTE for testing purposes this list is DUPLICATED in
    # CMake/Source/CMakeLists.txt, IF YOU CHANGE THIS LIST,
    # PLEASE UPDATE THAT FILE AS WELL!
    SET(CMAKE_Fortran_COMPILER_LIST
      ifort ifc efc f95 pgf95 lf95 xlf95 fort gfortran f90
      pgf90 xlf90 epcf90 fort77 frt pgf77 xlf fl32 af77 g77 f77
      )
  ENDIF(CMAKE_Fortran_COMPILER_INIT)

  # Find the compiler.
  FIND_PROGRAM(CMAKE_Fortran_COMPILER NAMES ${CMAKE_Fortran_COMPILER_LIST} DOC "Fortran compiler")
  IF(CMAKE_Fortran_COMPILER_INIT AND NOT CMAKE_Fortran_COMPILER)
    SET(CMAKE_Fortran_COMPILER "${CMAKE_Fortran_COMPILER_INIT}" CACHE FILEPATH "Fortran compiler" FORCE)
  ENDIF(CMAKE_Fortran_COMPILER_INIT AND NOT CMAKE_Fortran_COMPILER)
ENDIF(NOT CMAKE_Fortran_COMPILER)

MARK_AS_ADVANCED(CMAKE_Fortran_COMPILER)  

FIND_PROGRAM(CMAKE_AR NAMES ar )

FIND_PROGRAM(CMAKE_RANLIB NAMES ranlib)
IF(NOT CMAKE_RANLIB)
   SET(CMAKE_RANLIB : CACHE INTERNAL "noop for ranlib")
ENDIF(NOT CMAKE_RANLIB)
MARK_AS_ADVANCED(CMAKE_RANLIB)

# do not test for GNU if the generator is visual studio
IF(${CMAKE_GENERATOR} MATCHES "Visual Studio")
  SET(CMAKE_COMPILER_IS_GNUG77_RUN 1)
ENDIF(${CMAKE_GENERATOR} MATCHES "Visual Studio") 

IF(NOT CMAKE_COMPILER_IS_GNUG77_RUN)
  # test to see if the Fortran compiler is gnu
  
  IF(CMAKE_Fortran_FLAGS)
    SET(CMAKE_BOOT_Fortran_FLAGS ${CMAKE_Fortran_FLAGS})
  ELSE(CMAKE_Fortran_FLAGS)
    SET(CMAKE_BOOT_Fortran_FLAGS $ENV{FFLAGS})
  ENDIF(CMAKE_Fortran_FLAGS)
  EXEC_PROGRAM(${CMAKE_Fortran_COMPILER} ARGS ${CMAKE_BOOT_Fortran_FLAGS} -E "\"${CMAKE_ROOT}/Modules/CMakeTestGNU.c\"" OUTPUT_VARIABLE CMAKE_COMPILER_OUTPUT RETURN_VALUE CMAKE_COMPILER_RETURN)
  SET(CMAKE_COMPILER_IS_GNUG77_RUN 1)
  IF(NOT CMAKE_COMPILER_RETURN)
    IF("${CMAKE_COMPILER_OUTPUT}" MATCHES ".*THIS_IS_GNU.*" )
      SET(CMAKE_COMPILER_IS_GNUG77 1)
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Determining if the Fortran compiler is GNU succeeded with "
        "the following output:\n${CMAKE_COMPILER_OUTPUT}\n\n")
    ELSE("${CMAKE_COMPILER_OUTPUT}" MATCHES ".*THIS_IS_GNU.*" )
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Determining if the Fortran compiler is GNU failed with "
        "the following output:\n${CMAKE_COMPILER_OUTPUT}\n\n")
    ENDIF("${CMAKE_COMPILER_OUTPUT}" MATCHES ".*THIS_IS_GNU.*" )
    IF("${CMAKE_COMPILER_OUTPUT}" MATCHES ".*THIS_IS_MINGW.*" )
      SET(CMAKE_COMPILER_IS_MINGW 1)
    ENDIF("${CMAKE_COMPILER_OUTPUT}" MATCHES ".*THIS_IS_MINGW.*" )
    IF("${CMAKE_COMPILER_OUTPUT}" MATCHES ".*THIS_IS_CYGWIN.*" )
      SET(CMAKE_COMPILER_IS_CYGWIN 1)
    ENDIF("${CMAKE_COMPILER_OUTPUT}" MATCHES ".*THIS_IS_CYGWIN.*" )
  ENDIF(NOT CMAKE_COMPILER_RETURN)
ENDIF(NOT CMAKE_COMPILER_IS_GNUG77_RUN)


# configure variables set in this file for fast reload later on
CONFIGURE_FILE(${CMAKE_ROOT}/Modules/CMakeFortranCompiler.cmake.in 
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeFortranCompiler.cmake IMMEDIATE)
MARK_AS_ADVANCED(CMAKE_AR)
SET(CMAKE_Fortran_COMPILER_ENV_VAR "FC")
