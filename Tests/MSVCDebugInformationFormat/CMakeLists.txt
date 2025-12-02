cmake_minimum_required(VERSION 3.24)
cmake_policy(SET CMP0141 NEW)

# The debug information format flags do not change preprocessor definitions,
# so override our table of flags to artificially add a definition we can check.
set(CMAKE_USER_MAKE_RULES_OVERRIDE_C ${CMAKE_CURRENT_SOURCE_DIR}/override-C.cmake)
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/override-CXX.cmake)
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CUDA ${CMAKE_CURRENT_SOURCE_DIR}/override-CUDA.cmake)
set(CMAKE_USER_MAKE_RULES_OVERRIDE_Fortran ${CMAKE_CURRENT_SOURCE_DIR}/override-Fortran.cmake)

project(MSVCDebugInformationFormat)
if(CMake_TEST_CUDA)
  enable_language(CUDA)
endif()
if(CMake_TEST_Fortran)
  enable_language(Fortran)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if("${CMAKE_C_COMPILER_ID};${CMAKE_C_SIMULATE_ID};${CMAKE_C_COMPILER_FRONTEND_VARIANT}" STREQUAL "Clang;MSVC;GNU")
  set(verify_default VERIFY_Z7)
  set(NO_COMPILER_PDB 1)
elseif("${CMAKE_C_COMPILER_ID};${CMAKE_C_SIMULATE_ID};${CMAKE_C_COMPILER_FRONTEND_VARIANT}" STREQUAL "Clang;MSVC;MSVC")
  set(verify_default VERIFY_Zi)
  set(NO_EDIT_AND_CONTINUE 1)
else()
  set(verify_default VERIFY_Zi)
endif()

set(verify_def_Embedded        -DVERIFY_Z7)
set(verify_def_ProgramDatabase -DVERIFY_Zi)
set(verify_def_EditAndContinue -DVERIFY_ZI)

function(verify_combination format lang src)
  # Test that try_compile builds with this debug format.
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "${format}")
  set(CMAKE_TRY_COMPILE_CONFIGURATION "Debug")
  set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
  try_compile(${format}_COMPILES
    ${CMAKE_CURRENT_BINARY_DIR}/try_compile/${format}
    ${CMAKE_CURRENT_SOURCE_DIR}/${src}
    COMPILE_DEFINITIONS ${verify_def_${format}}
    CMAKE_FLAGS -DINCLUDE_DIRECTORIES=${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE ${format}_OUTPUT
    )
  if(${format}_COMPILES)
    message(STATUS "try_compile ${lang} with ${format} worked")
  else()
    string(REPLACE "\n" "\n  " ${format}_OUTPUT "  ${${format}_OUTPUT}")
    message(SEND_ERROR "try_compile ${lang} with ${format} failed:\n${${format}_OUTPUT}")
  endif()

  # Test that targets build with this debug format.
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<BOOL:$<TARGET_PROPERTY:BOOL_TRUE>>:${format}>$<$<BOOL:$<TARGET_PROPERTY:BOOL_FALSE>>:BadContent>")
  add_library(${format}-${lang} ${src})
  set_property(TARGET ${format}-${lang} PROPERTY BOOL_TRUE TRUE)
  target_compile_definitions(${format}-${lang} PRIVATE ${verify_def_${format}})
endfunction()

function(verify lang src)
  add_library(default-${lang} ${src})
  target_compile_definitions(default-${lang} PRIVATE "$<$<CONFIG:Debug,RelWithDebInfo>:${verify_default}>")

  verify_combination(Embedded ${lang} ${src})
  if(NOT NO_COMPILER_PDB)
    verify_combination(ProgramDatabase ${lang} ${src})
    if(NOT NO_EDIT_AND_CONTINUE AND NOT lang MATCHES "^(Fortran)$")
      verify_combination(EditAndContinue ${lang} ${src})
    endif()
  endif()
endfunction()

verify(C verify.c)
verify(CXX verify.cxx)
if(CMake_TEST_CUDA)
  verify(CUDA verify.cu)
endif()
if(CMake_TEST_Fortran)
  verify(Fortran verify.F90)
endif()
