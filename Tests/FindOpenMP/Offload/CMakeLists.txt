cmake_minimum_required(VERSION 4.0)
project(TestFindOpenMPOffload NONE)
include(CTest)

foreach(c C CXX Fortran)
  if(OpenMP_TEST_OFFLOAD_${c})
    enable_language(${c})
    list(APPEND langs ${c})
    # Offload is enabled here as a builder would; the targets below opt in
    # by linking the offload target.
    set(OpenMP_${c}_OFFLOAD ON)
  endif()
endforeach()

find_package(OpenMP REQUIRED COMPONENTS ${langs})

set(main_C main.c)
set(offload_C offload.c)
if("CXX" IN_LIST langs)
  configure_file(main.c main.cxx COPYONLY)
  configure_file(offload.c offload.cxx COPYONLY)
  set(main_CXX ${CMAKE_CURRENT_BINARY_DIR}/main.cxx)
  set(offload_CXX ${CMAKE_CURRENT_BINARY_DIR}/offload.cxx)
endif()
if("Fortran" IN_LIST langs)
  if(OpenMP_Fortran_HAVE_OMPLIB_MODULE)
    set(OpenMP_Fortran_INCLUDE_LINE "use omp_lib\n      implicit none")
  else()
    set(OpenMP_Fortran_INCLUDE_LINE "implicit none\n      include 'omp_lib.h'")
  endif()
  configure_file(offload.f90.in offload.f90 @ONLY)
  set(main_Fortran main.f90)
  set(offload_Fortran ${CMAKE_CURRENT_BINARY_DIR}/offload.f90)
endif()

# Each compiler handles the offload flags for its own language.
foreach(c IN LISTS langs)
  add_executable(offload_exe_${c} ${main_${c}} ${offload_${c}})
  target_link_libraries(offload_exe_${c} PRIVATE OpenMP::OpenMP_${c}_Offload)
  list(APPEND tests exe_${c})
endforeach()

# How the flags reach the link that embeds the device code doesn't depend
# on the language, so the library layouts are only built for C.
if("C" IN_LIST langs)
  # Static library: the flags reach the executable's link through the
  # library's usage requirements.
  add_library(offload_lib STATIC offload.c)
  target_link_libraries(offload_lib PRIVATE OpenMP::OpenMP_C_Offload)
  add_executable(offload_static main.c)
  target_link_libraries(offload_static PRIVATE offload_lib)

  # Shared library: its own link embeds the device code.
  add_library(offload_dso SHARED offload.c)
  target_link_libraries(offload_dso PRIVATE OpenMP::OpenMP_C_Offload)
  add_executable(offload_shared main.c)
  target_link_libraries(offload_shared PRIVATE offload_dso)
  if(CMAKE_C_COMPILER_ID STREQUAL "NVHPC")
    # Without -mp=gpu, NVHPC links host-only stubs of its GPU runtime into
    # the executable, and they shadow the ones the library needs.
    target_link_libraries(offload_shared PRIVATE
      "$<LINK_ONLY:OpenMP::OpenMP_C_Offload>")
  endif()

  list(APPEND tests static shared)

  if("CXX" IN_LIST langs AND CMAKE_C_COMPILER_ID STREQUAL CMAKE_CXX_COMPILER_ID)
    # C kernel library linked by the C++ compiler driver.
    add_executable(offload_mixed ${main_CXX})
    target_link_libraries(offload_mixed PRIVATE offload_lib)
    list(APPEND tests mixed)
  endif()
endif()

foreach(t IN LISTS tests)
  add_test(NAME offload_${t} COMMAND offload_${t})
  # A silent fallback to the host fails the test instead of passing it.
  set_property(TEST offload_${t} PROPERTY ENVIRONMENT OMP_TARGET_OFFLOAD=MANDATORY)
endforeach()
