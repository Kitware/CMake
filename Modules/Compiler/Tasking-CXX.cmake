# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.
include(Compiler/Tasking)
__compiler_tasking(CXX)

set(CMAKE_CXX98_STANDARD_COMPILE_OPTION "--c++=03" "--strict")
set(CMAKE_CXX98_EXTENSION_COMPILE_OPTION "--iso=03" " ")

set(CMAKE_CXX11_STANDARD_COMPILE_OPTION "--c++=11" "--strict")
set(CMAKE_CXX11_EXTENSION_COMPILE_OPTION "--c++=11" " ")

set(CMAKE_CXX14_STANDARD_COMPILE_OPTION "--c++=14" "--strict")
set(CMAKE_CXX14_EXTENSION_COMPILE_OPTION "--c++=14" " ")

if(CMAKE_CXX_COMPILER_ARCHITECTURE_ID STREQUAL "TriCore")
  if(CMAKE_TASKING_TOOLSET STREQUAL "SmartCode")
    __compiler_check_default_language_standard(CXX 10.1 14)
  else()
    __compiler_check_default_language_standard(CXX 6.3 14)
  endif()
elseif(CMAKE_CXX_COMPILER_ARCHITECTURE_ID STREQUAL "ARM")
  if(CMAKE_TASKING_TOOLSET STREQUAL "SmartCode")
    __compiler_check_default_language_standard(CXX 10.1 14)
  elseif(CMAKE_TASKING_TOOLSET STREQUAL "TriCore")
    __compiler_check_default_language_standard(CXX 6.3 14)
  else()
    __compiler_check_default_language_standard(CXX 6.0 14)
  endif()
else()
  message(FATAL_ERROR "CXX is not supported with the ${CMAKE_CXX_COMPILER_ARCHITECTURE_ID} architecture.")
endif()
