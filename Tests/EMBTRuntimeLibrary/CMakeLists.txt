cmake_minimum_required(VERSION 3.28)
project(EMBTRuntimeLibrary C CXX)

# Verify that an EMBT_RUNTIME_LIBRARY selection compiles and defines the expected
# macros.  The selection is applied both as a directory variable (which
# initializes the target property and is forwarded into try_compile) and directly
# on a target, so both paths are exercised.  The VERIFY_* switches arm the #error
# guards in verify.h, making a wrong property->flag mapping a build failure.
function(verify tag lang src)
  cmake_parse_arguments(PARSE_ARGV 3 A "" "PROP;VALUE;TYPE" "DEFS")
  if(NOT A_TYPE)
    set(A_TYPE STATIC)
  endif()

  # Local to this function scope, so it does not leak to sibling cases.
  if(A_PROP)
    set(CMAKE_${A_PROP} "${A_VALUE}")
  endif()

  # try_compile wants -D-prefixed definitions; target_compile_definitions wants
  # bare names.
  set(try_defs "")
  foreach(d IN LISTS A_DEFS)
    list(APPEND try_defs "-D${d}")
  endforeach()

  # try_compile only supports EXECUTABLE / STATIC_LIBRARY target types, so the
  # shared-library case is covered by the real add_library() target below.
  if(NOT A_TYPE STREQUAL "SHARED")
    set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
    try_compile(${tag}_${lang}_COMPILES
      ${CMAKE_CURRENT_BINARY_DIR}/tc/${tag}-${lang}
      ${CMAKE_CURRENT_SOURCE_DIR}/${src}
      COMPILE_DEFINITIONS ${try_defs}
      OUTPUT_VARIABLE out)
    if(${tag}_${lang}_COMPILES)
      message(STATUS "try_compile ${tag} (${lang}) worked")
    else()
      string(REPLACE "\n" "\n  " out "  ${out}")
      message(SEND_ERROR "try_compile ${tag} (${lang}) failed:\n${out}")
    endif()
  endif()

  add_library(${tag}-${lang} ${A_TYPE} ${src})
  if(A_PROP)
    set_property(TARGET ${tag}-${lang} PROPERTY ${A_PROP} "${A_VALUE}")
  endif()
  target_compile_definitions(${tag}-${lang} PRIVATE ${A_DEFS})
endfunction()

foreach(lang_src "C;verify.c" "CXX;verify.cxx")
  list(GET lang_src 0 lang)
  list(GET lang_src 1 src)

  # No selection: none of the macros are defined.
  verify(default ${lang} ${src})

  # Runtime library.
  verify(static  ${lang} ${src} PROP EMBT_RUNTIME_LIBRARY VALUE Static)
  verify(dynamic ${lang} ${src} PROP EMBT_RUNTIME_LIBRARY VALUE Dynamic
         DEFS VERIFY_RTLDLL VERIFY_USEPACKAGES)

  # A shared library is a plain DLL: linked with -tD and compiled with __DLL__.
  # A trivial DLL links without the RAD Studio runtime.
  verify(dll ${lang} ${src} TYPE SHARED DEFS VERIFY_DLL)
endforeach()
