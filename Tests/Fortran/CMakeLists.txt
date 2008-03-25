cmake_minimum_required (VERSION 2.6)
PROJECT(testf Fortran)
SET(CMAKE_VERBOSE_MAKEFILE 1)
MESSAGE("ENV_FLAGS = $ENV{FFLAGS}")
MESSAGE("CMAKE_Fortran_COMPILER_INIT = ${CMAKE_Fortran_COMPILER_INIT}")
MESSAGE("CMAKE_Fortran_COMPILER_FULLPATH = ${CMAKE_Fortran_COMPILER_FULLPATH}")
MESSAGE("CMAKE_Fortran_COMPILER = ${CMAKE_Fortran_COMPILER}")
MESSAGE("CMAKE_Fortran_FLAGS = ${CMAKE_Fortran_FLAGS}")
ADD_EXECUTABLE(testf hello.f)

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
