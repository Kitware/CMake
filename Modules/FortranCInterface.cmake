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
#  discover_fortran_module_mangling - try different types of 
#  fortran modle name mangling to find one that works
#
#
#
# this function tests a single fortran mangling.  
# CODE - test code to try should define a subroutine called "sub"
# PREFIX - string to put in front of sub
# POSTFIX - string to put after sub
# ISUPPER - if TRUE then sub will be called as SUB
# DOC - string used in status checking Fortran ${DOC} linkage
# SUB - the name of the SUB to call
# RESULT place to store result TRUE if this linkage works, FALSE
#        if not.
#
function(test_fortran_mangling CODE PREFIX ISUPPER POSTFIX DOC SUB RESULT)
  if(ISUPPER)
    string(TOUPPER "${SUB}" sub)
  else(ISUPPER) 
    string(TOLOWER "${SUB}" sub)
  endif(ISUPPER)
  set(FUNCTION "${PREFIX}${sub}${POSTFIX}")
  # create a fortran file with sub called sub
  # 
  set(TMP_DIR
    "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckFortranLink")
  file(REMOVE_RECURSE "${TMP_DIR}")
  file(WRITE "${TMP_DIR}/test.f" "${CODE}"    )
  message(STATUS "checking Fortran ${DOC} linkage: ${FUNCTION}")
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

# this function discovers the name mangling scheme used
# for functions in a fortran module.  
function(discover_fortran_module_mangling prefix suffix found)
  set(CODE 
    "
      module test_interface
      interface dummy
         module procedure sub
      end interface
      contains
        subroutine sub
        end subroutine
      end module test_interface
    ")
  set(worked FALSE)
  foreach(interface 
      "test_interface$" 
      "TEST_INTERFACE_mp_" 
      "_test_interface__" 
      "__test_interface__" 
      "__test_interface_NMOD_" 
      "__test_interface_MOD_")
    test_fortran_mangling("${CODE}" "${interface}"
      ${FORTRAN_C_MANGLING_UPPERCASE} "" "module" "sub" worked)
    if(worked)
      # if this is the upper case module match then
      # lower case it for the extraction of pre and post strings
      if("${interface}" MATCHES "TEST_INTERFACE")
        string(TOLOWER "${interface}" interface)
      endif()
      string(REGEX REPLACE "(.*)test_interface(.*)" "\\1" pre "${interface}")
      string(REGEX REPLACE "(.*)test_interface(.*)" "\\2" post "${interface}")
      set(${prefix} "${pre}" PARENT_SCOPE)
      set(${suffix} "${post}" PARENT_SCOPE)
      set(${found} TRUE PARENT_SCOPE)
      return()
    endif(worked)
  endforeach(interface)
  if(NOT worked)
    message(STATUS "Failed to find C binding to Fortran module functions.")
    set(${prefix} "BROKEN_C_FORTRAN_MODULE_BINDING" PARENT_SCOPE)
    set(${suffix} "BROKEN_C_FORTRAN_MODULE_BINDING" PARENT_SCOPE)
    set(${found} FALSE PARENT_SCOPE)
  endif(NOT worked)
endfunction(discover_fortran_module_mangling)


function(discover_fortran_mangling prefix isupper suffix extra_under_score
    found )
  set(CODE 
    "
      subroutine sub
      end subroutine sub
    ")
  foreach(post "_" "")
    foreach(isup FALSE TRUE)
      foreach(pre "" "_" "__")
        set(worked FALSE)
        test_fortran_mangling("${CODE}" "${pre}" ${isup}
          "${post}" "function" sub worked )
        if(worked)
          message(STATUS "found Fortran function linkage")
          set(${isupper} "${isup}" PARENT_SCOPE)
          set(${prefix} "${pre}" PARENT_SCOPE)
          set(${suffix} "${post}" PARENT_SCOPE)
          set(${found} TRUE PARENT_SCOPE)
          set(CODE 
            "
      subroutine my_sub
      end subroutine my_sub
    ")
          set(worked FALSE)
          test_fortran_mangling("${CODE}" "${pre}" ${isup}
            "${post}" "function with _ " my_sub worked )
          if(worked)
            set(${extra_under_score} FALSE PARENT_SCOPE)
          else(worked)
            test_fortran_mangling("${CODE}" "${pre}" ${isup}
              "${post}_" "function with _ " my_sub worked )
            if(worked)
              set(${extra_under_score} TRUE PARENT_SCOPE)
            endif(worked)
          endif(worked)
        return()
        endif()
      endforeach()
    endforeach()
  endforeach()
  set(${found} FALSE PARENT_SCOPE)
endfunction(discover_fortran_mangling)

function(create_fortran_c_interface NAMESPACE FUNCTIONS HEADER)
  if(NOT FORTRAN_C_MANGLING_FOUND)
    # find regular fortran function mangling
    discover_fortran_mangling(prefix isupper suffix extra_under found)
    if(NOT found)
      message(SEND_ERROR "Could not find fortran c name mangling.")
      return()
    endif(NOT found)
    # find fortran module function mangling
    set(FORTRAN_C_PREFIX "${prefix}" CACHE INTERNAL
      "PREFIX for Fortran to c name mangling")
    set(FORTRAN_C_SUFFIX "${suffix}" CACHE INTERNAL
      "SUFFIX for Fortran to c name mangling")
    set(FORTRAN_C_MANGLING_UPPERCASE ${isupper} CACHE INTERNAL 
      "Was fortran to c mangling found" )
    set(FORTRAN_C_MANGLING_EXTRA_UNDERSCORE ${extra_under} CACHE INTERNAL 
      "If a function has a _ in the name does the compiler append an extra _" )
    set(FORTRAN_C_MANGLING_FOUND TRUE CACHE INTERNAL 
      "Was fortran to c mangling found" )
    set(prefix )
    set(suffix )
    set(found FALSE)
    # only try this if the compiler is F90 compatible
    if(CMAKE_Fortran_COMPILER_SUPPORTS_F90)
      discover_fortran_module_mangling(prefix suffix found)
    endif(CMAKE_Fortran_COMPILER_SUPPORTS_F90)
    if(found)
      message(STATUS "found Fortran module linkage")
    else(found)
      message(STATUS "Failed to find Fortran module linkage")
    endif(found)
    set(FORTRAN_C_MODULE_PREFIX "${prefix}" CACHE INTERNAL
      "PREFIX for Fortran to c name mangling")
    set(FORTRAN_C_MODULE_SUFFIX "${suffix}" CACHE INTERNAL
      "SUFFIX for Fortran to c name mangling")
    set(FORTRAN_C_MODULE_MANGLING_FOUND ${found} CACHE INTERNAL
      "Was for Fortran to c name mangling found for modules")
  endif(NOT FORTRAN_C_MANGLING_FOUND)
  foreach(f ${${FUNCTIONS}})
    if(FORTRAN_C_MANGLING_UPPERCASE)
      string(TOUPPER "${f}" fcase)
    else()
      string(TOLOWER "${f}" fcase)
    endif()
    if("${f}" MATCHES ":")
      string(REGEX REPLACE "(.*):(.*)" "\\1" module "${f}")
      string(REGEX REPLACE "(.*):(.*)" "\\2" function "${f}")
      string(REGEX REPLACE "(.*):(.*)" "\\1" module_case "${fcase}")
      string(REGEX REPLACE "(.*):(.*)" "\\2" function_case "${fcase}")
      set(HEADER_CONTENT "${HEADER_CONTENT}
#define ${NAMESPACE}${module}_${function} ${FORTRAN_C_MODULE_PREFIX}${module_case}${FORTRAN_C_MODULE_SUFFIX}${function_case}
")
    else("${f}" MATCHES ":")
      set(function "${FORTRAN_C_PREFIX}${fcase}${FORTRAN_C_SUFFIX}")
      if("${f}" MATCHES "_" AND FORTRAN_C_MANGLING_EXTRA_UNDERSCORE)
        set(function "${function}_")
      endif("${f}" MATCHES "_" AND FORTRAN_C_MANGLING_EXTRA_UNDERSCORE)
      set(HEADER_CONTENT "${HEADER_CONTENT}
#define ${NAMESPACE}${f} ${function}
")
    endif("${f}" MATCHES ":")
  endforeach(f)
  configure_file(
    "${CMAKE_ROOT}/Modules/FortranCInterface.h.in"
    ${HEADER} @ONLY)
  message(STATUS "created ${HEADER}")
endfunction()

