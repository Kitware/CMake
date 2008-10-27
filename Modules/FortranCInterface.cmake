# FortranCInterface.cmake
#
# This file defines the function create_fortran_c_interface.
# this function is used to create a configured header file 
# that contains a mapping from C to a Fortran function using
# the correct name mangling scheme as defined by the current 
# fortran compiler.  
#
# The function tages a list of functions and the name of 
# a header file to configure.  
#
# This file also defines some helper functions that are used
# to detect the fortran name mangling scheme used by the 
# current Fortran compiler.
#  test_fortran_mangling - test a single fortran mangling 
#  discover_fortran_mangling - loop over all combos of fortran
#   name mangling and call test_fortran_mangling until one of them
#   works.
#

function(test_fortran_mangling PREFIX ISUPPER POSTFIX RESULT)
  if(ISUPPER)
    set(FUNCTION "${PREFIX}SUB${POSTFIX}")
  else(ISUPPER)
    set(FUNCTION "${PREFIX}sub${POSTFIX}")
  endif(ISUPPER)
  # create a fortran file with sub called sub
  # 
  set(TMP_DIR
    "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckFortranLink")
  file(REMOVE_RECURSE "${TMP_DIR}")
  file(WRITE "${TMP_DIR}/test.f"
    "
      subroutine sub
      end subroutine sub
    "
    )
  message(STATUS "checking Fortran linkage: ${FUNCTION}")
  file(WRITE "${TMP_DIR}/ctof.c"
    "
      extern ${FUNCTION}();
      int main() { ${FUNCTION}(); return 0;}
    "
    )
  file(WRITE "${TMP_DIR}/CMakeLists.txt"
    "
     project(testf C Fortran)
     add_library(flib test.f)
     add_executable(ctof ctof.c)
     target_link_libraries(ctof flib)
    "
    )
  set(FORTRAN_NAME_MANGLE_TEST FALSE)
  try_compile(FORTRAN_NAME_MANGLE_TEST "${TMP_DIR}" "${TMP_DIR}"
    testf
    OUTPUT_VARIABLE output)
  if(FORTRAN_NAME_MANGLE_TEST)
    set(${RESULT} TRUE PARENT_SCOPE)
  else()
    set(${RESULT} FALSE PARENT_SCOPE)
  endif()
endfunction(test_fortran_mangling)

function(discover_fortran_mangling prefix isupper suffix found )
  foreach(pre "_" "" "__")
    foreach(isup TRUE FALSE)
      foreach(post "" "_")
        set(worked FALSE)
        test_fortran_mangling("${pre}" ${isup} "${post}" worked )
        if(worked)
          message(STATUS "found Fotran linkage")
          set(${isupper} "${isup}" PARENT_SCOPE)
          set(${prefix} "${pre}" PARENT_SCOPE)
          set(${suffix} "${post}" PARENT_SCOPE)
          set(${found} TRUE PARENT_SCOPE)
          return()
        endif()
      endforeach()
    endforeach()
  endforeach()
  set(${found} FALSE PARENT_SCOPE)
endfunction(discover_fortran_mangling)

function(create_fortran_c_interface NAMESPACE FUNCTIONS HEADER)
  if(NOT FORTRAN_C_MANGLING_FOUND)
    discover_fortran_mangling(prefix isupper suffix found)
    if(NOT found)
      message(SEND_ERROR "Could not find fortran c name mangling.")
      return()
    endif(NOT found)
    set(FORTRAN_C_PREFIX "${prefix}" CACHE INTERNAL
      "PREFIX for Fortran to c name mangling")
    set(FORTRAN_C_SUFFIX "${suffix}" CACHE INTERNAL
      "SUFFIX for Fortran to c name mangling")
    set(FORTRAN_C_MANGLING_UPPERCASE ${isupper} CACHE INTERNAL 
      "Was fortran to c mangling found" )
    set(FORTRAN_C_MANGLING_FOUND TRUE CACHE INTERNAL 
      "Was fortran to c mangling found" )
  endif(NOT FORTRAN_C_MANGLING_FOUND)
  foreach(f ${${FUNCTIONS}})
    if(${FORTRAN_C_MANGLING_UPPERCASE})
      string(TOUPPER "${f}" ff)
    else()
      string(TOLOWER "${f}" ff)
    endif()
    set(function "${FORTRAN_C_PREFIX}${ff}${FORTRAN_C_SUFFIX}")
    set(HEADER_CONTENT "${HEADER_CONTENT}
#define ${NAMESPACE}${f} ${function}
")
  endforeach(f)
  configure_file(
    "${CMAKE_ROOT}/Modules/FortranCInterface.h.in"
    ${HEADER} @ONLY)
  message(STATUS "created ${HEADER}")
endfunction()

#  TODO
# need to add support for module linking
# module test_interface
#
#    interface dummy
#        module procedure module_function
#    end interface
#
# contains
#
#    subroutine module_function
#    end subroutine
#
# end module test_interface
#
# produces this:
# __test_interface_MOD_module_function
#
# 
