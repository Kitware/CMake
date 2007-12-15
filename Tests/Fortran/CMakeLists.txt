PROJECT(testf Fortran)
SET(CMAKE_VERBOSE_MAKEFILE 1)
MESSAGE("ENV_FLAGS = $ENV{FFLAGS}")
MESSAGE("CMAKE_Fortran_COMPILER_INIT = ${CMAKE_Fortran_COMPILER_INIT}")
MESSAGE("CMAKE_Fortran_COMPILER_FULLPATH = ${CMAKE_Fortran_COMPILER_FULLPATH}")
MESSAGE("CMAKE_Fortran_COMPILER = ${CMAKE_Fortran_COMPILER}")
MESSAGE("CMAKE_Fortran_FLAGS = ${CMAKE_Fortran_FLAGS}")
ADD_EXECUTABLE(testf hello.f)

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

  ADD_SUBDIRECTORY(Library)
ENDIF(CMAKE_Fortran_COMPILER_SUPPORTS_F90)
