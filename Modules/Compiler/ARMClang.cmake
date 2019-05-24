if(_ARMClang_CMAKE_LOADED)
  return()
endif()
set(_ARMClang_CMAKE_LOADED TRUE)

cmake_policy(PUSH)
cmake_policy(SET CMP0057 NEW) # if IN_LIST

get_filename_component(_CMAKE_C_TOOLCHAIN_LOCATION "${CMAKE_C_COMPILER}" PATH)
get_filename_component(_CMAKE_CXX_TOOLCHAIN_LOCATION "${CMAKE_CXX_COMPILER}" PATH)

set(CMAKE_EXECUTABLE_SUFFIX ".elf")

find_program(CMAKE_ARMClang_LINKER armlink HINTS "${_CMAKE_C_TOOLCHAIN_LOCATION}" "${_CMAKE_CXX_TOOLCHAIN_LOCATION}" )
find_program(CMAKE_ARMClang_AR     armar   HINTS "${_CMAKE_C_TOOLCHAIN_LOCATION}" "${_CMAKE_CXX_TOOLCHAIN_LOCATION}" )

set(CMAKE_LINKER "${CMAKE_ARMClang_LINKER}" CACHE FILEPATH "The ARMClang linker" FORCE)
mark_as_advanced(CMAKE_ARMClang_LINKER)
set(CMAKE_AR "${CMAKE_ARMClang_AR}" CACHE FILEPATH "The ARMClang archiver" FORCE)
mark_as_advanced(CMAKE_ARMClang_AR)

# get compiler supported cpu list
function(__armclang_set_processor_list lang out_var)
  execute_process(COMMAND "${CMAKE_${lang}_COMPILER}" --target=${CMAKE_${lang}_COMPILER_TARGET} -mcpu=list
    OUTPUT_VARIABLE processor_list
    ERROR_VARIABLE processor_list)
  string(REGEX MATCHALL "-mcpu=([^ \n]*)" processor_list "${processor_list}")
  string(REGEX REPLACE "-mcpu=" "" processor_list "${processor_list}")
  set(${out_var} "${processor_list}" PARENT_SCOPE)
endfunction()

# check processor is in list
function(__armclang_check_processor processor list out_var)
  string(TOLOWER "${processor}" processor)
  if(processor IN_LIST list)
    set(${out_var} TRUE PARENT_SCOPE)
  else()
    set(${out_var} FALSE PARENT_SCOPE)
  endif()
endfunction()

macro(__compiler_armclang lang)
  if(NOT CMAKE_${lang}_COMPILER_TARGET)
    set(CMAKE_${lang}_COMPILER_TARGET arm-arm-none-eabi)
  endif()
  if(NOT CMAKE_${lang}_COMPILER_PROCESSOR_LIST)
    __armclang_set_processor_list(${lang} CMAKE_${lang}_COMPILER_PROCESSOR_LIST)
  endif()
  if(NOT CMAKE_SYSTEM_PROCESSOR)
    message(FATAL_ERROR "  CMAKE_SYSTEM_PROCESSOR must be set for ARMClang\n"
      "  Supported processor: ${CMAKE_${lang}_COMPILER_PROCESSOR_LIST}\n")
  else()
    __armclang_check_processor("${CMAKE_SYSTEM_PROCESSOR}" "${CMAKE_${lang}_COMPILER_PROCESSOR_LIST}" _CMAKE_${lang}_CHECK_RESULT)
    if(NOT _CMAKE_${lang}_CHECK_RESULT)
      message(FATAL_ERROR "  System processor '${CMAKE_SYSTEM_PROCESSOR}' not supported by ARMClang ${lang} compiler\n"
        "  Supported processor: ${CMAKE_${lang}_COMPILER_PROCESSOR_LIST}\n")
    endif()
    unset(_CMAKE_${lang}_CHECK_RESULT)
  endif()
  string(APPEND CMAKE_${lang}_FLAGS_INIT "-mcpu=${CMAKE_SYSTEM_PROCESSOR}")
  string(APPEND CMAKE_${lang}_LINK_FLAGS "--cpu=${CMAKE_SYSTEM_PROCESSOR}")

  set(CMAKE_${lang}_LINK_EXECUTABLE "<CMAKE_LINKER> <CMAKE_${lang}_LINK_FLAGS> <LINK_FLAGS> <LINK_LIBRARIES> <OBJECTS> -o <TARGET> --list <TARGET_BASE>.map")
  set(CMAKE_${lang}_CREATE_STATIC_LIBRARY  "<CMAKE_AR> --create -cr <TARGET> <LINK_FLAGS> <OBJECTS>")
  set(CMAKE_${lang}_ARCHIVE_CREATE         "<CMAKE_AR> --create -cr <TARGET> <LINK_FLAGS> <OBJECTS>")
  set(CMAKE_${lang}_RESPONSE_FILE_LINK_FLAG "--via=")
  set(CMAKE_${lang}_OUTPUT_EXTENSION ".o")
  set(CMAKE_${lang}_OUTPUT_EXTENSION_REPLACE 1)
endmacro()

cmake_policy(POP)
