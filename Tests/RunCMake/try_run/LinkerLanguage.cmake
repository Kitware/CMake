enable_language(CXX)
enable_language(Fortran)

set (lib_name "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}lib${CMAKE_STATIC_LIBRARY_SUFFIX}")
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set (undef_flag -u _func)
  else()
    set (undef_flag -u func)
  endif()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set (undef_flag -u _func)
else()
  set (undef_flag -u func)
endif()

set(CMAKE_TRY_COMPILE_TARGET_TYPE EXECUTABLE)
try_run(run_result compile_result
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/lib.cxx ${CMAKE_CURRENT_SOURCE_DIR}/main.f90
  COMPILE_OUTPUT_VARIABLE compile_out
  RUN_OUTPUT_VARIABLE run_out
  LINKER_LANGUAGE Fortran)

if(NOT compile_result)
  message(FATAL_ERROR "try_run(... LINKER_LANGUAGE Fortran) compilation failed:\n${compile_out}")
endif()
if(run_result STREQUAL "FAILED_TO_RUN")
  message(FATAL_ERROR "try_run(... LINKER_LANGUAGE Fortran) execution failed:\n${run_out}")
endif()
