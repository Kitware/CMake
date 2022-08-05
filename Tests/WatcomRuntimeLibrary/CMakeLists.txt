cmake_minimum_required(VERSION 3.23)
cmake_policy(SET CMP0136 NEW)
project(WatcomRuntimeLibrary)

function(verify_combinations threads lang src)
  set(verify_def_MultiThreaded -DVERIFY_MT)
  set(verify_def_DLL -DVERIFY_DLL)
  foreach(dll "" DLL)
    # Construct the name of this runtime library combination.
    set(rtl "${threads}${dll}")

    # Test that try_compile builds with this RTL.
    set(CMAKE_WATCOM_RUNTIME_LIBRARY "${rtl}")
    set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
    try_compile(${rtl}_COMPILES
      ${CMAKE_CURRENT_BINARY_DIR}/try_compile/${rtl}
      ${CMAKE_CURRENT_SOURCE_DIR}/${src}
      COMPILE_DEFINITIONS ${verify_def_${threads}} ${verify_def_${dll}}
      OUTPUT_VARIABLE ${rtl}_OUTPUT
      )
    if(${rtl}_COMPILES)
      message(STATUS "try_compile with ${rtl} worked")
    else()
      string(REPLACE "\n" "\n  " ${rtl}_OUTPUT "  ${${rtl}_OUTPUT}")
      message(SEND_ERROR "try_compile with ${rtl} failed:\n${${rtl}_OUTPUT}")
    endif()

    # Test that targets build with this RTL.
    set(CMAKE_WATCOM_RUNTIME_LIBRARY "$<$<BOOL:$<TARGET_PROPERTY:BOOL_TRUE>>:${rtl}>$<$<BOOL:$<TARGET_PROPERTY:BOOL_FALSE>>:BadContent>")
    add_library(${rtl}-${lang} ${src})
    set_property(TARGET ${rtl}-${lang} PROPERTY BOOL_TRUE TRUE)
    target_compile_definitions(${rtl}-${lang} PRIVATE ${verify_def_${threads}} ${verify_def_${dll}})
  endforeach()
endfunction()

function(verify lang src)
  add_library(default-${lang} ${src})
  target_compile_definitions(default-${lang} PRIVATE VERIFY_MT VERIFY_DLL)

  verify_combinations(SingleThreaded ${lang} ${src})
  verify_combinations(MultiThreaded ${lang} ${src})

  # Test known  default behavior when no flag is given.
  set(CMAKE_WATCOM_RUNTIME_LIBRARY "")
  add_library(empty-${lang} ${src})
endfunction()

verify(C verify.c)
verify(CXX verify.cxx)
