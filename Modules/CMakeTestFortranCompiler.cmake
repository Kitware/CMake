# This file is used by EnableLanguage in cmGlobalGenerator to
# determine that that selected Fortran compiler can actually compile
# and link the most basic of programs.   If not, a fatal error
# is set and cmake stops processing commands and will not generate
# any makefiles or projects.
IF(NOT CMAKE_Fortran_COMPILER_WORKS)
  MESSAGE(STATUS "Check for working Fortran compiler: ${CMAKE_Fortran_COMPILER}")
  FILE(WRITE ${CMAKE_BINARY_DIR}/CMakeTmp/testFortranCompiler.f "
        PROGRAM TESTFortran
        PRINT *, 'Hello'
        END
  ")
  TRY_COMPILE(CMAKE_Fortran_COMPILER_WORKS ${CMAKE_BINARY_DIR} 
    ${CMAKE_BINARY_DIR}/CMakeTmp/testFortranCompiler.f
    OUTPUT_VARIABLE OUTPUT)
ENDIF(NOT CMAKE_Fortran_COMPILER_WORKS)

IF(NOT CMAKE_Fortran_COMPILER_WORKS)
  MESSAGE(STATUS "Check for working Fortran compiler: ${CMAKE_Fortran_COMPILER} -- broken")
  FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeError.log
    "Determining if the Fortran compiler works failed with "
    "the following output:\n${OUTPUT}\n\n")
  MESSAGE(FATAL_ERROR "The Fortran compiler \"${CMAKE_Fortran_COMPILER}\" "
    "is not able to compile a simple test program.\nIt fails "
    "with the following output:\n ${OUTPUT}\n\n"
    "CMake will not be able to correctly generate this project.")
ELSE(NOT CMAKE_Fortran_COMPILER_WORKS)
  MESSAGE(STATUS "Check for working Fortran compiler: ${CMAKE_Fortran_COMPILER} -- works")
  FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
    "Determining if the Fortran compiler works passed with "
    "the following output:\n${OUTPUT}\n\n")
ENDIF(NOT CMAKE_Fortran_COMPILER_WORKS)
