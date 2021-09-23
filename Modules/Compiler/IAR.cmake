# This file is processed when the IAR C/C++ Compiler is used
#
# CPU <arch> supported in CMake: 8051, Arm, AVR, MSP430, RH850, RISC-V, RL78, RX and V850
#
# The compiler user documentation is architecture-dependent
# and it can found with the product installation under <arch>/doc/{EW,BX}<arch>_DevelopmentGuide.ENU.pdf
#
#
include_guard()

macro(__compiler_iar_ilink lang)
  set(CMAKE_EXECUTABLE_SUFFIX ".elf")
  set(CMAKE_${lang}_OUTPUT_EXTENSION ".o")
  if (${lang} STREQUAL "C" OR ${lang} STREQUAL "CXX")
    set(CMAKE_${lang}_COMPILE_OBJECT             "<CMAKE_${lang}_COMPILER> ${CMAKE_IAR_${lang}_FLAG} --silent <SOURCE> <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT>")
    set(CMAKE_${lang}_CREATE_PREPROCESSED_SOURCE "<CMAKE_${lang}_COMPILER> ${CMAKE_IAR_${lang}_FLAG} --silent <SOURCE> <DEFINES> <INCLUDES> <FLAGS> --preprocess=cnl <PREPROCESSED_SOURCE>")
    set(CMAKE_${lang}_CREATE_ASSEMBLY_SOURCE     "<CMAKE_${lang}_COMPILER> ${CMAKE_IAR_${lang}_FLAG} --silent <SOURCE> <DEFINES> <INCLUDES> <FLAGS> -lAH <ASSEMBLY_SOURCE> -o <OBJECT>.dummy")

    set(CMAKE_${lang}_RESPONSE_FILE_LINK_FLAG "-f ")
    set(CMAKE_DEPFILE_FLAGS_${lang} "--dependencies=ns <DEP_FILE>")

    string(APPEND CMAKE_${lang}_FLAGS_INIT " ")
    string(APPEND CMAKE_${lang}_FLAGS_DEBUG_INIT " -r")
    string(APPEND CMAKE_${lang}_FLAGS_MINSIZEREL_INIT " -Ohz -DNDEBUG")
    string(APPEND CMAKE_${lang}_FLAGS_RELEASE_INIT " -Oh -DNDEBUG")
    string(APPEND CMAKE_${lang}_FLAGS_RELWITHDEBINFO_INIT " -Oh -r -DNDEBUG")
  endif()

  set(CMAKE_${lang}_LINK_EXECUTABLE "<CMAKE_LINKER> --silent <OBJECTS> <CMAKE_${lang}_LINK_FLAGS> <LINK_FLAGS> <LINK_LIBRARIES> -o <TARGET>")
  set(CMAKE_${lang}_CREATE_STATIC_LIBRARY "<CMAKE_AR> <TARGET> --create <LINK_FLAGS> <OBJECTS>")
  set(CMAKE_${lang}_ARCHIVE_CREATE "<CMAKE_AR> <TARGET> --create <LINK_FLAGS> <OBJECTS>")
  set(CMAKE_${lang}_ARCHIVE_APPEND "<CMAKE_AR> <TARGET> --replace <LINK_FLAGS> <OBJECTS>")
  set(CMAKE_${lang}_ARCHIVE_FINISH "")
endmacro()

macro(__compiler_iar_xlink lang)
  set(CMAKE_EXECUTABLE_SUFFIX ".bin")
  if (${lang} STREQUAL "C" OR ${lang} STREQUAL "CXX")

    set(CMAKE_${lang}_COMPILE_OBJECT             "<CMAKE_${lang}_COMPILER> ${CMAKE_IAR_${lang}_FLAG} --silent <SOURCE> <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT>")
    set(CMAKE_${lang}_CREATE_PREPROCESSED_SOURCE "<CMAKE_${lang}_COMPILER> ${CMAKE_IAR_${lang}_FLAG} --silent <SOURCE> <DEFINES> <INCLUDES> <FLAGS> --preprocess=cnl <PREPROCESSED_SOURCE>")
    set(CMAKE_${lang}_CREATE_ASSEMBLY_SOURCE     "<CMAKE_${lang}_COMPILER> ${CMAKE_IAR_${lang}_FLAG} --silent <SOURCE> <DEFINES> <INCLUDES> <FLAGS> -lAH <ASSEMBLY_SOURCE> -o <OBJECT>.dummy")

    set(CMAKE_${lang}_RESPONSE_FILE_LINK_FLAG "-f ")
    set(CMAKE_DEPFILE_FLAGS_${lang} "--dependencies=ns <DEP_FILE>")

    string(APPEND CMAKE_${lang}_FLAGS_INIT " ")
    string(APPEND CMAKE_${lang}_FLAGS_DEBUG_INIT " -r")
    string(APPEND CMAKE_${lang}_FLAGS_MINSIZEREL_INIT " -Ohz -DNDEBUG")
    string(APPEND CMAKE_${lang}_FLAGS_RELEASE_INIT " -Oh -DNDEBUG")
    string(APPEND CMAKE_${lang}_FLAGS_RELWITHDEBINFO_INIT " -Oh -r -DNDEBUG")
  endif()

  set(CMAKE_${lang}_LINK_EXECUTABLE "<CMAKE_LINKER> -S <OBJECTS> <CMAKE_${lang}_LINK_FLAGS> <LINK_FLAGS> <LINK_LIBRARIES> -o <TARGET>")
  set(CMAKE_${lang}_CREATE_STATIC_LIBRARY "<CMAKE_AR> <TARGET> <LINK_FLAGS> <OBJECTS>")
  set(CMAKE_${lang}_ARCHIVE_CREATE "<CMAKE_AR> <TARGET> <LINK_FLAGS> <OBJECTS>")
  set(CMAKE_${lang}_ARCHIVE_APPEND "")
  set(CMAKE_${lang}_ARCHIVE_FINISH "")

  set(CMAKE_LIBRARY_PATH_FLAG "-I")
endmacro()
