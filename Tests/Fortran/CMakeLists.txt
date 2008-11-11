cmake_minimum_required (VERSION 2.6)
project(testf Fortran C)
message("CTEST_FULL_OUTPUT ")
set(CMAKE_VERBOSE_MAKEFILE 1)
message("ENV_FLAGS = $ENV{FFLAGS}")
message("CMAKE_Fortran_COMPILER_INIT = ${CMAKE_Fortran_COMPILER_INIT}")
message("CMAKE_Fortran_COMPILER_FULLPATH = ${CMAKE_Fortran_COMPILER_FULLPATH}")
message("CMAKE_Fortran_COMPILER = ${CMAKE_Fortran_COMPILER}")
message("CMAKE_Fortran_FLAGS = ${CMAKE_Fortran_FLAGS}")
add_executable(testf hello.f)


function(test_fortran_c_interface_module)
  message(STATUS "Testing FortranCInterface module")
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
  create_fortran_c_interface("F_" FORTRAN_FUNCTIONS 
    "${testf_BINARY_DIR}/foo.h")
  include_directories("${testf_BINARY_DIR}")
  
  # if the name mangling is not found for a F90 compiler
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
  message("Fortran = ${CMAKE_Fortran_COMPILER_ID}")
  message("C = ${CMAKE_C_COMPILER_ID}")
  add_executable(foo ${srcs})
  if(NOT ("$ENV{EXTRA_FORTRAN_C_LIBS}" STREQUAL ""))
    message(STATUS 
      "linking in extra C to Fortran libs $ENV{EXTRA_FORTRAN_C_LIBS}")
    target_link_libraries(foo $ENV{EXTRA_FORTRAN_C_LIBS})
  endif()
  # print out some stuff to help debug on machines via cdash
  file(READ "${testf_BINARY_DIR}/foo.h" fooh)
  message("foo.h contents:\n${fooh}")
endfunction()

# if the id's match or the compilers are compatible, then
# call the test_fortran_c_interface_module function
if(("${CMAKE_Fortran_COMPILER_ID}" MATCHES "Intel")
    AND 
    ("${CMAKE_C_COMPILER_ID}" MATCHES "MSVC")
    )
  set(COMPATABLE_COMPILERS TRUE)
endif()
if(COMPATABLE_COMPILERS
    OR ("${CMAKE_Fortran_COMPILER_ID}" MATCHES "${CMAKE_C_COMPILER_ID}" ))
  test_fortran_c_interface_module()
else()
  message("Fortran does not match c compiler")
  message("Fortran = ${CMAKE_Fortran_COMPILER_ID}")
  message("C = ${CMAKE_C_COMPILER_ID}")
  # hack to make g77 work after CL has been enabled 
  # as a languge, cmake needs language specific versions
  # of these variables....
  if(WIN32 AND "${CMAKE_Fortran_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_CREATE_CONSOLE_EXE )
    set(CMAKE_LIBRARY_PATH_FLAG "-L")
    set(CMAKE_LINK_LIBRARY_FLAG "-l")
    set(CMAKE_LINK_LIBRARY_SUFFIX )
  endif()
  # gnu and sunpro do not use the same flags here...
  # however if LDFLAGS is used to set -m64 it causes odd stuf
  # with the fortran build
  if( ("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
      AND ("${CMAKE_Fortran_COMPILER_ID}" MATCHES "SunPro"))
    set(CMAKE_EXE_LINKER_FLAGS "")
    set(CMAKE_Fortran_FLAGS "")
  endif()

endif()




set(TEST_MODULE_DEPENDS 0)
if(CMAKE_Fortran_COMPILER_SUPPORTS_F90)
  add_executable(test_module
    test_module_main.f90
    test_module_implementation.f90
    test_module_interface.f90)

  add_executable(test_use_in_comment_fixedform
    test_use_in_comment_fixedform.f)
  add_executable(test_use_in_comment_freeform 
    test_use_in_comment_freeform.f90)

  add_executable(test_in_interface 
    in_interface/main.f90
    in_interface/module.f90)

  add_definitions(-DFOO -DBAR=1)
  add_executable(test_preprocess test_preprocess.F90)

  set(TEST_MODULE_DEPENDS 1)
endif(CMAKE_Fortran_COMPILER_SUPPORTS_F90)

if(TEST_MODULE_DEPENDS)
  # Build the external project separately using a custom target.
  # Make sure it uses the same build configuration as this test.
  if(CMAKE_CONFIGURATION_TYPES)
    set(External_CONFIG_TYPE -C "${CMAKE_CFG_INTDIR}")
  else(CMAKE_CONFIGURATION_TYPES)
    set(External_CONFIG_TYPE)
  endif(CMAKE_CONFIGURATION_TYPES)
  add_custom_command(
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
  add_custom_target(ExternalTarget ALL DEPENDS ${testf_BINARY_DIR}/ExternalProject)

  # Test module output directory if available.
  if(CMAKE_Fortran_MODDIR_FLAG)
    set(Library_MODDIR "${testf_BINARY_DIR}/Library/modules")
  else(CMAKE_Fortran_MODDIR_FLAG)
    set(Library_MODDIR "${testf_BINARY_DIR}/Library")
  endif(CMAKE_Fortran_MODDIR_FLAG)

  add_subdirectory(Library)
  add_subdirectory(Executable)
endif(TEST_MODULE_DEPENDS)
