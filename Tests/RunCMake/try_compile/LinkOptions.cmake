enable_language(C)

set (lib_name "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}lib${CMAKE_STATIC_LIBRARY_SUFFIX}")
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  if (RunCMake_C_COMPILER_ID STREQUAL "MSVC"
      OR ("x${CMAKE_C_SIMULATE_ID}" STREQUAL "xMSVC" AND
          NOT CMAKE_C_COMPILER_ID STREQUAL "Clang" OR "x${CMAKE_C_COMPILER_FRONTEND_VARIANT}" STREQUAL "xMSVC"))
    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
      set (undef_flag /INCLUDE:_func)
    else()
      set (undef_flag /INCLUDE:func)
    endif()
  else()
    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
      set (undef_flag -u _func)
    else()
      set (undef_flag -u func)
    endif()
  endif()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set (undef_flag -u _func)
else()
  set (undef_flag -u func)
endif()

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
try_compile(result ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/lib.c
  COPY_FILE "${lib_name}")

set(CMAKE_TRY_COMPILE_TARGET_TYPE EXECUTABLE)
try_compile(result ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/main.c
  OUTPUT_VARIABLE out
  LINK_OPTIONS ${undef_flag} "${lib_name}")

if(NOT result)
  message(FATAL_ERROR "try_compile(... LINK_OPTIONS ...)  failed:\n${out}")
endif()
