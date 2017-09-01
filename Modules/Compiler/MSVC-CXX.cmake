# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

include(Compiler/CMakeCommonCompilerMacros)

if ((CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.0.24215.1 AND
     CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.10) OR
   CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.10.25017)

  # VS 2015 Update 3 and above support language standard level flags,
  # with the default and minimum level being C++14.
  set(CMAKE_CXX98_STANDARD_COMPILE_OPTION "")
  set(CMAKE_CXX98_EXTENSION_COMPILE_OPTION "")
  set(CMAKE_CXX11_STANDARD_COMPILE_OPTION "")
  set(CMAKE_CXX11_EXTENSION_COMPILE_OPTION "")
  set(CMAKE_CXX14_STANDARD_COMPILE_OPTION "-std:c++14")
  set(CMAKE_CXX14_EXTENSION_COMPILE_OPTION "-std:c++14")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.11.25505)
    set(CMAKE_CXX17_STANDARD_COMPILE_OPTION "-std:c++17")
    set(CMAKE_CXX17_EXTENSION_COMPILE_OPTION "-std:c++17")
  else()
    set(CMAKE_CXX17_STANDARD_COMPILE_OPTION "-std:c++latest")
    set(CMAKE_CXX17_EXTENSION_COMPILE_OPTION "-std:c++latest")
  endif()

  __compiler_check_default_language_standard(CXX 19.0 14)
elseif (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0)
  # MSVC has no specific options to set language standards, but set them as
  # empty strings anyways so the feature test infrastructure can at least check
  # to see if they are defined.
  set(CMAKE_CXX98_STANDARD_COMPILE_OPTION "")
  set(CMAKE_CXX98_EXTENSION_COMPILE_OPTION "")
  set(CMAKE_CXX11_STANDARD_COMPILE_OPTION "")
  set(CMAKE_CXX11_EXTENSION_COMPILE_OPTION "")
  set(CMAKE_CXX14_STANDARD_COMPILE_OPTION "")
  set(CMAKE_CXX14_EXTENSION_COMPILE_OPTION "")
  set(CMAKE_CXX17_STANDARD_COMPILE_OPTION "")
  set(CMAKE_CXX17_EXTENSION_COMPILE_OPTION "")

  # There is no meaningful default for this
  set(CMAKE_CXX_STANDARD_DEFAULT "")
endif()
