
# This file is used by EnableLanguage in cmGlobalGenerator to
# determine that that selected C compiler can actually compile
# and link the most basic of programs.   If not, a fatal error
# is set and cmake stops processing commands and will not generate
# any makefiles or projects.
IF(NOT CMAKE_C_COMPILER_WORKS)
  MESSAGE(STATUS "Check for working C compiler: ${CMAKE_C_COMPILER}")
  FILE(WRITE ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testCCompiler.c
    "#ifdef __cplusplus\n"
    "# error \"The CMAKE_C_COMPILER is set to a C++ compiler\"\n"
    "#endif\n"
    "#include <stdio.h>\n"
    "#if defined(__CLASSIC_C__)\n"
    "int main(argc, argv)\n"
    "  int argc;\n"
    "  char* argv[];\n"
    "#else\n"
    "int main(int argc, char* argv[])\n"
    "#endif\n"
    "{ printf(\"%s\\n\", argv[0]); return argc-1;}\n")
  TRY_COMPILE(CMAKE_C_COMPILER_WORKS ${CMAKE_BINARY_DIR} 
    ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testCCompiler.c
    OUTPUT_VARIABLE OUTPUT) 
  SET(C_TEST_WAS_RUN 1)
ENDIF(NOT CMAKE_C_COMPILER_WORKS)

IF(NOT CMAKE_C_COMPILER_WORKS)
  MESSAGE(STATUS "Check for working C compiler: ${CMAKE_C_COMPILER} -- broken")
  FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
    "Determining if the C compiler works failed with "
    "the following output:\n${OUTPUT}\n\n")
  MESSAGE(FATAL_ERROR "The C compiler \"${CMAKE_C_COMPILER}\" "
    "is not able to compile a simple test program.\nIt fails "
    "with the following output:\n ${OUTPUT}\n\n"
    "CMake will not be able to correctly generate this project.")
ELSE(NOT CMAKE_C_COMPILER_WORKS)
  IF(C_TEST_WAS_RUN)
    MESSAGE(STATUS "Check for working C compiler: ${CMAKE_C_COMPILER} -- works")
    FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "Determining if the C compiler works passed with "
      "the following output:\n${OUTPUT}\n\n") 
  ENDIF(C_TEST_WAS_RUN)
  INCLUDE (${CMAKE_ROOT}/Modules/CheckTypeSize.cmake)
  # Check the size of void*.  This used to be "void *" but icc expands the *.
  CHECK_TYPE_SIZE("void*"  CMAKE_SIZEOF_VOID_P)
  SET(CMAKE_C_COMPILER_WORKS 1 CACHE INTERNAL "")
  # re-configure this file CMakeCCompiler.cmake so that it gets
  # the value for CMAKE_SIZEOF_VOID_P
  # configure variables set in this file for fast reload later on
  CONFIGURE_FILE(${CMAKE_ROOT}/Modules/CMakeCCompiler.cmake.in 
    ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeCCompiler.cmake IMMEDIATE)
ENDIF(NOT CMAKE_C_COMPILER_WORKS)

