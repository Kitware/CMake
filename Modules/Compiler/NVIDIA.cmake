
# This module is shared by multiple languages; use include blocker.
if(__COMPILER_NVIDIA)
  return()
endif()
set(__COMPILER_NVIDIA 1)

include(Compiler/CMakeCommonCompilerMacros)

macro(__compiler_nvidia_cxx_standards lang)
  if("x${CMAKE_${lang}_SIMULATE_ID}" STREQUAL "xMSVC")
    # MSVC requires c++14 as the minimum level
    set(CMAKE_${lang}03_STANDARD_COMPILE_OPTION "")
    set(CMAKE_${lang}03_EXTENSION_COMPILE_OPTION "")

    # MSVC requires c++14 as the minimum level
    set(CMAKE_${lang}11_STANDARD_COMPILE_OPTION "")
    set(CMAKE_${lang}11_EXTENSION_COMPILE_OPTION "")

    if (NOT CMAKE_${lang}_COMPILER_VERSION VERSION_LESS 9.0)
      if(CMAKE_${lang}_SIMULATE_VERSION VERSION_GREATER_EQUAL 19.10.25017)
        set(CMAKE_${lang}14_STANDARD_COMPILE_OPTION "-std=c++14")
        set(CMAKE_${lang}14_EXTENSION_COMPILE_OPTION "-std=c++14")
      else()
        set(CMAKE_${lang}14_STANDARD_COMPILE_OPTION "")
        set(CMAKE_${lang}14_EXTENSION_COMPILE_OPTION "")
      endif()
    endif()

    if (NOT CMAKE_${lang}_COMPILER_VERSION VERSION_LESS 11.0)
      if(CMAKE_${lang}_SIMULATE_VERSION VERSION_GREATER_EQUAL 19.11.25505)
        set(CMAKE_${lang}17_STANDARD_COMPILE_OPTION "-std=c++17")
        set(CMAKE_${lang}17_EXTENSION_COMPILE_OPTION "-std=c++17")
      endif()
    endif()

    if (NOT CMAKE_${lang}_COMPILER_VERSION VERSION_LESS 12.0)
      if(CMAKE_${lang}_SIMULATE_VERSION VERSION_GREATER_EQUAL 19.11.25505)
        set(CMAKE_${lang}20_STANDARD_COMPILE_OPTION "-std=c++20")
        set(CMAKE_${lang}20_EXTENSION_COMPILE_OPTION "-std=c++20")
      endif()
    endif()
  else()
    set(CMAKE_${lang}03_STANDARD_COMPILE_OPTION "")
    set(CMAKE_${lang}03_EXTENSION_COMPILE_OPTION "")

    set(CMAKE_${lang}11_STANDARD_COMPILE_OPTION "-std=c++11")
    set(CMAKE_${lang}11_EXTENSION_COMPILE_OPTION "-std=c++11")

    if (NOT CMAKE_${lang}_COMPILER_VERSION VERSION_LESS 9.0)
      set(CMAKE_${lang}03_STANDARD_COMPILE_OPTION "-std=c++03")
      set(CMAKE_${lang}03_EXTENSION_COMPILE_OPTION "-std=c++03")
      set(CMAKE_${lang}14_STANDARD_COMPILE_OPTION "-std=c++14")
      set(CMAKE_${lang}14_EXTENSION_COMPILE_OPTION "-std=c++14")
    endif()

    if (NOT CMAKE_${lang}_COMPILER_VERSION VERSION_LESS 11.0)
      set(CMAKE_${lang}17_STANDARD_COMPILE_OPTION "-std=c++17")
      set(CMAKE_${lang}17_EXTENSION_COMPILE_OPTION "-std=c++17")
    endif()

    if (NOT CMAKE_${lang}_COMPILER_VERSION VERSION_LESS 12.0)
      set(CMAKE_${lang}20_STANDARD_COMPILE_OPTION "-std=c++20")
      set(CMAKE_${lang}20_EXTENSION_COMPILE_OPTION "-std=c++20")
    endif()
  endif()

  __compiler_check_default_language_standard(${lang} 6.0 03)
endmacro()
