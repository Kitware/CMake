cmake_minimum_required(VERSION 3.14)
cmake_policy(SET CMP0091 NEW)
project(MSVCRuntimeLibraryFortran Fortran)

foreach(t MultiThreaded SingleThreaded)
  foreach(dbg "" Debug)
    foreach(dll "" DLL)
      set(var "CMAKE_Fortran_COMPILE_OPTIONS_MSVC_RUNTIME_LIBRARY_${t}${dbg}${dll}")
      if(CMAKE_Fortran_COMPILER_ID STREQUAL "Intel")
        # ifort does not actually define these, so inject them
        string(REPLACE "-threads" "-threads;-D_MT" "${var}" "${${var}}")
        string(REPLACE "-dbglibs" "-dbglibs;-D_DEBUG" "${var}" "${${var}}")
      elseif(CMAKE_Fortran_COMPILER_ID STREQUAL "Flang")
        # flang does not actually define these, so inject them
        string(REPLACE ";--dependent-lib=libcmt;" ";--dependent-lib=libcmt;-D_MT;" "${var}" ";${${var}};")
        string(REPLACE ";--dependent-lib=msvcrt;" ";--dependent-lib=msvcrt;-D_MT;-D_DLL;" "${var}" ";${${var}};")
        string(REPLACE ";--dependent-lib=libcmtd;" ";--dependent-lib=libcmtd;-D_MT;-D_DEBUG;" "${var}" ";${${var}};")
        string(REPLACE ";--dependent-lib=msvcrtd;" ";--dependent-lib=msvcrtd;-D_MT;-D_DEBUG;-D_DLL;" "${var}" ";${${var}};")
      endif()
    endforeach()
  endforeach()
endforeach()
string(APPEND CMAKE_Fortran_FLAGS " -w")

function(verify_combinations threads lang src)
  set(verify_tc_config_ Release)
  set(verify_tc_config_Debug Debug)
  set(verify_def_MultiThreaded -DVERIFY_MT)
  set(verify_def_Debug -DVERIFY_DEBUG)
  set(verify_def_DLL -DVERIFY_DLL)
  foreach(dbg "" Debug)
    foreach(dll "" DLL)
      # Construct the name of this runtime library combination.
      set(rtl "${threads}${dbg}${dll}")

      # Test that targets build with this RTL.
      set(CMAKE_MSVC_RUNTIME_LIBRARY "$<$<BOOL:$<TARGET_PROPERTY:BOOL_TRUE>>:${rtl}>$<$<BOOL:$<TARGET_PROPERTY:BOOL_FALSE>>:BadContent>")
      add_library(${rtl}-${lang} ${src})
      set_property(TARGET ${rtl}-${lang} PROPERTY BOOL_TRUE TRUE)
      target_compile_definitions(${rtl}-${lang} PRIVATE ${verify_def_${threads}} ${verify_def_${dbg}} ${verify_def_${dll}})
    endforeach()
  endforeach()
endfunction()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

function(verify lang src)
  add_library(default-${lang} ${src})
  target_compile_definitions(default-${lang} PRIVATE VERIFY_MT VERIFY_DLL "$<$<CONFIG:Debug>:VERIFY_DEBUG>")
  verify_combinations(MultiThreaded ${lang} ${src})
endfunction()

verify(Fortran verify.F90)
# Intel Fortran for Windows supports single-threaded RTL but it is
# not implemented by the Visual Studio integration.
if(CMAKE_Fortran_COMPILER_ID STREQUAL "Intel" AND NOT CMAKE_GENERATOR MATCHES "Visual Studio")
  verify_combinations(SingleThreaded Fortran verify.F90)
endif()
