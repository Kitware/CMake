cmake_minimum_required(VERSION 3.31)
cmake_policy(SET CMP0184 NEW)

# All runtime checks flags enables single preprocessor definition,
# so override our table of flags to artificially add a definition we can check.
set(CMAKE_USER_MAKE_RULES_OVERRIDE_C ${CMAKE_CURRENT_SOURCE_DIR}/override-C.cmake)
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/override-CXX.cmake)
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CUDA ${CMAKE_CURRENT_SOURCE_DIR}/override-CUDA.cmake)
set(CMAKE_USER_MAKE_RULES_OVERRIDE_Fortran ${CMAKE_CURRENT_SOURCE_DIR}/override-Fortran.cmake)

project(MSVCRuntimeChecks)
if(CMake_TEST_CUDA STREQUAL "NVIDIA")
  enable_language(CUDA)
endif()
if(CMake_TEST_Fortran)
  enable_language(Fortran)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(verify_default VERIFY_RTCsu)

set(verify_def_PossibleDataLoss      -DVERIFY_RTCc)
set(verify_def_StackFrameErrorCheck  -DVERIFY_RTCs)
set(verify_def_UninitializedVariable -DVERIFY_RTCu)
set(verify_def_RTCsu                 -DVERIFY_RTCsu)

function(verify_combination format verify_format_defs lang src)
  # Test that try_compile builds with this runtime check.
  set(CMAKE_MSVC_RUNTIME_CHECKS "${format}")
  set(CMAKE_TRY_COMPILE_CONFIGURATION "Debug")
  set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

  string(REPLACE ";" "_" format_var_name "${format}")
  if (NOT format_var_name)
    set(format_var_name "Empty")
  endif()

  if (NOT verify_format_defs)
    foreach(format_for_def IN LISTS format)
      list(APPEND verify_format_defs ${verify_def_${format_for_def}})
    endforeach()
  endif()

  try_compile(${format_var_name}_COMPILES
    ${CMAKE_CURRENT_BINARY_DIR}/try_compile/${format_var_name}
    ${CMAKE_CURRENT_SOURCE_DIR}/${src}
    COMPILE_DEFINITIONS ${verify_format_defs}
    CMAKE_FLAGS -DINCLUDE_DIRECTORIES=${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE ${format_var_name}_OUTPUT
    )
  if(${format_var_name}_COMPILES)
    message(STATUS "try_compile ${lang} with \"${format}\" worked")
  else()
    string(REPLACE "\n" "\n  " ${format_var_name}_OUTPUT "  ${${format_var_name}_OUTPUT}")
    message(SEND_ERROR "try_compile ${lang} with \"${format}\" failed:\n${${format_var_name}_OUTPUT}")
  endif()

  # Test that targets build with this runtime check.
  set(CMAKE_MSVC_RUNTIME_CHECKS "$<$<BOOL:$<TARGET_PROPERTY:BOOL_TRUE>>:${format}>$<$<BOOL:$<TARGET_PROPERTY:BOOL_FALSE>>:BadContent>")
  add_library(${format_var_name}-${lang} ${src})
  set_property(TARGET ${format_var_name}-${lang} PROPERTY BOOL_TRUE TRUE)
  target_compile_definitions(${format_var_name}-${lang} PRIVATE ${verify_format_defs})
endfunction()

function(verify lang src)
  add_library(default-${lang} ${src})
  target_compile_definitions(default-${lang} PRIVATE "$<$<CONFIG:Debug>:${verify_default}>")

  # zero checkers
  verify_combination("" "" ${lang} ${src})

  # single checker
  verify_combination(PossibleDataLoss "" ${lang} ${src})
  verify_combination(StackFrameErrorCheck "" ${lang} ${src})
  verify_combination(UninitializedVariable "" ${lang} ${src})
  verify_combination(RTCsu "" ${lang} ${src})

  # multiple checkers (without RTCsu merging)
  verify_combination("PossibleDataLoss;StackFrameErrorCheck" "" ${lang} ${src})
  verify_combination("PossibleDataLoss;UninitializedVariable" "" ${lang} ${src})

  # multiple checkers (only RTCsu merging)
  set(defs "${verify_def_RTCsu}")
  verify_combination("StackFrameErrorCheck;UninitializedVariable" "${defs}" ${lang} ${src})
  verify_combination("StackFrameErrorCheck;RTCsu" "${defs}" ${lang} ${src})
  verify_combination("UninitializedVariable;RTCsu" "${defs}" ${lang} ${src})
  verify_combination("StackFrameErrorCheck;UninitializedVariable;RTCsu" "${defs}" ${lang} ${src})

  # multiple checkers (with RTCsu merging)
  list(APPEND defs "${verify_def_PossibleDataLoss}")
  verify_combination("PossibleDataLoss;StackFrameErrorCheck;UninitializedVariable" "${defs}" ${lang} ${src})
  verify_combination("PossibleDataLoss;StackFrameErrorCheck;RTCsu" "${defs}" ${lang} ${src})
  verify_combination("PossibleDataLoss;UninitializedVariable;RTCsu" "${defs}" ${lang} ${src})
  verify_combination("PossibleDataLoss;StackFrameErrorCheck;UninitializedVariable;RTCsu" "${defs}" ${lang} ${src})
endfunction()

verify(C verify.c)
verify(CXX verify.cxx)
if(CMake_TEST_CUDA STREQUAL "NVIDIA")
  verify(CUDA verify.cu)
endif()
if(CMake_TEST_Fortran)
  verify(Fortran verify.F90)
endif()
