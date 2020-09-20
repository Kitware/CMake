include(Compiler/TI)
__compiler_ti(C)

# Architecture specific
# C99 versions: https://processors.wiki.ti.com/index.php/C99_Support_in_TI_Compilers

if("${CMAKE_C_COMPILER_ARCHITECTURE_ID}" STREQUAL "ARM")
  set(__COMPILER_TI_C99_VERSION_ARM 5.2)

elseif("${CMAKE_C_COMPILER_ARCHITECTURE_ID}" STREQUAL "MSP430")
  set(__COMPILER_TI_C99_VERSION_MSP430 4.3)

elseif("${CMAKE_C_COMPILER_ARCHITECTURE_ID}" STREQUAL "TMS320C28x")
  set(__COMPILER_TI_C99_VERSION_TMS320C28x 6.3)

elseif("${CMAKE_C_COMPILER_ARCHITECTURE_ID}" STREQUAL "TMS320C6x")
  set(__COMPILER_TI_C99_VERSION_TMS320C6x 7.5)

else()
  # architecture not handled
  return()

endif()


if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL "${__COMPILER_TI_C99_VERSION_${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}}")

  set(CMAKE_C90_STANDARD_COMPILE_OPTION "--c89" "--strict_ansi")
  set(CMAKE_C90_EXTENSION_COMPILE_OPTION "--c89" "--relaxed_ansi")

  set(CMAKE_C99_STANDARD_COMPILE_OPTION "--c99" "--strict_ansi")
  set(CMAKE_C99_EXTENSION_COMPILE_OPTION "--c99" "--relaxed_ansi")

else()

  set(CMAKE_C90_STANDARD_COMPILE_OPTION "--strict_ansi")
  set(CMAKE_C90_EXTENSION_COMPILE_OPTION "--relaxed_ansi")

endif()
