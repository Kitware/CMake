cmake_minimum_required (VERSION 2.6)
PROJECT(testf C Fortran)
SET(CMAKE_VERBOSE_MAKEFILE 1)
MESSAGE("ENV_FLAGS = $ENV{FFLAGS}")
MESSAGE("CMAKE_Fortran_COMPILER_INIT = ${CMAKE_Fortran_COMPILER_INIT}")
MESSAGE("CMAKE_Fortran_COMPILER_FULLPATH = ${CMAKE_Fortran_COMPILER_FULLPATH}")
MESSAGE("CMAKE_Fortran_COMPILER = ${CMAKE_Fortran_COMPILER}")
MESSAGE("CMAKE_Fortran_FLAGS = ${CMAKE_Fortran_FLAGS}")
ADD_EXECUTABLE(testf hello.f)

# test the C to Fortran interface module
include(FortranCInterface)
if(CMAKE_Fortran_COMPILER_SUPPORTS_F90)
  if(FORTRAN_C_MODULE_MANGLING_FOUND)
    set(srcs foo.f)
    set(FORTRAN_FUNCTIONS test_mod:sub)
  else(FORTRAN_C_MODULE_MANGLING_FOUND)
    if(CMAKE_Fortran_COMPILER_ID MATCHES "SunPro|MIPSpro")
      message("${CMAKE_Fortran_COMPILER_ID} compilers do not support"
        " linking Fortran module procedures from C")
    else()
      message("This will fail, but let the user see the error")
      set(srcs foo.f)
      set(FORTRAN_FUNCTIONS test_mod:sub)
    endif()
  endif(FORTRAN_C_MODULE_MANGLING_FOUND)
endif()
set(FORTRAN_FUNCTIONS ${FORTRAN_FUNCTIONS} my_sub mysub )
set(srcs ${srcs} mysub.f foo.c)
create_fortran_c_interface("F_" FORTRAN_FUNCTIONS "${testf_BINARY_DIR}/foo.h")
include_directories("${testf_BINARY_DIR}")
add_executable(foo ${srcs})
# print out some stuff to help debug on machines via cdash
message("CTEST_FULL_OUTPUT ")
file(READ "${testf_BINARY_DIR}/foo.h" fooh)
message("foo.h contents:\n${fooh}")

# if the name mangling is not found for an F90 compiler
# print out some diagnostic stuff for the dashboard
if(CMAKE_Fortran_COMPILER_SUPPORTS_F90)
  if(NOT FORTRAN_C_MODULE_MANGLING_FOUND)
    file(GLOB_RECURSE O_OBJFILES
      "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckFortranLink/*.o"
      "*.o" )
    file(GLOB_RECURSE OBJ_OBJFILES 
      "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckFortranLink/*.obj")
    find_program(DUMPBIN dumpbin)
    find_program(NM nm)
    foreach(ofile ${O_OBJFILES} ${OBJ_OBJFILES})
      if(DEPENDS)
        execute_process(COMMAND ${DUMPBIN} /symbols "${ofile}" 
          OUTPUT_VARIABLE out)
        message("symbols in ${ofile}:\n${out}")
      endif()
      if(NM)
        execute_process(COMMAND ${NM} "${ofile}" OUTPUT_VARIABLE out)
        message("symbols in ${ofile}:\n${out}")
      endif()
    endforeach()
  endif()
endif()



SET(TEST_MODULE_DEPENDS 0)
IF(CMAKE_Fortran_COMPILER_SUPPORTS_F90)
  ADD_EXECUTABLE(test_module
    test_module_main.f90
    test_module_implementation.f90
    test_module_interface.f90)

  ADD_EXECUTABLE(test_use_in_comment_fixedform
    test_use_in_comment_fixedform.f)
  ADD_EXECUTABLE(test_use_in_comment_freeform 
    test_use_in_comment_freeform.f90)

  ADD_EXECUTABLE(test_in_interface 
    in_interface/main.f90
    in_interface/module.f90)

  ADD_DEFINITIONS(-DFOO -DBAR=1)
  ADD_EXECUTABLE(test_preprocess test_preprocess.F90)

  SET(TEST_MODULE_DEPENDS 1)
ENDIF(CMAKE_Fortran_COMPILER_SUPPORTS_F90)

IF(TEST_MODULE_DEPENDS)
  # Build the external project separately using a custom target.
  # Make sure it uses the same build configuration as this test.
  IF(CMAKE_CONFIGURATION_TYPES)
    SET(External_CONFIG_TYPE -C "${CMAKE_CFG_INTDIR}")
  ELSE(CMAKE_CONFIGURATION_TYPES)
    SET(External_CONFIG_TYPE)
  ENDIF(CMAKE_CONFIGURATION_TYPES)
  ADD_CUSTOM_COMMAND(
    OUTPUT ${testf_BINARY_DIR}/ExternalProject
    COMMAND ${CMAKE_CTEST_COMMAND}
    ARGS ${External_CONFIG_TYPE}
         --build-and-test
         ${testf_SOURCE_DIR}/External
         ${testf_BINARY_DIR}/External
         --build-noclean
         --build-two-config
         --build-project ExtFort
         --build-generator ${CMAKE_GENERATOR}
         --build-makeprogram ${CMAKE_MAKE_PROGRAM}
         --build-options -DCMAKE_Fortran_COMPILER:STRING=${CMAKE_Fortran_COMPILER}
                         -DCMAKE_Fortran_FLAGS:STRING=${CMAKE_Fortran_FLAGS}
                         -DCMAKE_Fortran_FLAGS_DEBUG:STRING=${CMAKE_Fortran_FLAGS_DEBUG}
                         -DCMAKE_Fortran_FLAGS_RELEASE:STRING=${CMAKE_Fortran_FLAGS_RELEASE}
                         -DCMAKE_Fortran_FLAGS_MINSIZEREL:STRING=${CMAKE_Fortran_FLAGS_MINSIZEREL}
                         -DCMAKE_Fortran_FLAGS_RELWITHDEBINFO:STRING=${CMAKE_Fortran_FLAGS_RELWITHDEBINFO}
         )
  ADD_CUSTOM_TARGET(ExternalTarget ALL DEPENDS ${testf_BINARY_DIR}/ExternalProject)

  # Test module output directory if available.
  IF(CMAKE_Fortran_MODDIR_FLAG)
    SET(Library_MODDIR "${testf_BINARY_DIR}/Library/modules")
  ELSE(CMAKE_Fortran_MODDIR_FLAG)
    SET(Library_MODDIR "${testf_BINARY_DIR}/Library")
  ENDIF(CMAKE_Fortran_MODDIR_FLAG)

  ADD_SUBDIRECTORY(Library)
  ADD_SUBDIRECTORY(Executable)
ENDIF(TEST_MODULE_DEPENDS)
