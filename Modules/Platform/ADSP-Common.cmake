include_guard()

macro(__platform_adsp_init)
  if(NOT CMAKE_ADSP_PLATFORM_INITIALIZED)
    if(NOT CMAKE_SYSTEM_PROCESSOR)
      message(FATAL_ERROR "ADSP: CMAKE_SYSTEM_PROCESSOR is required but not set")
    endif()

    set(CMAKE_ADSP_PROCESSOR "ADSP-${CMAKE_SYSTEM_PROCESSOR}")
    string(TOUPPER "${CMAKE_ADSP_PROCESSOR}" CMAKE_ADSP_PROCESSOR)

    set(CMAKE_ADSP_COMPILER_NAME cc21k.exe)
    if(CMAKE_ADSP_PROCESSOR MATCHES "^ADSP-BF")
      set(CMAKE_ADSP_COMPILER_NAME ccblkfn.exe)
    endif()

    set(CMAKE_ADSP_PLATFORM_INITIALIZED TRUE)
  endif()
endmacro()

macro(__platform_adsp lang)
  __platform_adsp_init()
  set(CMAKE_${lang}_COMPILER "${CMAKE_ADSP_ROOT}/${CMAKE_ADSP_COMPILER_NAME}")

  execute_process(
    COMMAND "${CMAKE_${lang}_COMPILER}" "-proc=${CMAKE_ADSP_PROCESSOR}" "-version"
    OUTPUT_QUIET ERROR_QUIET
    RESULT_VARIABLE _adsp_is_valid_proc
  )
  if(NOT _adsp_is_valid_proc EQUAL 0)
    message(FATAL_ERROR
      "ADSP: unsupported processor '${CMAKE_ADSP_PROCESSOR}' for CMAKE_${lang}_COMPILER:\n"
      "  ${CMAKE_${lang}_COMPILER}"
    )
  endif()
endmacro()
