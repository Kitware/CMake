# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

include (Compiler/PellesC)
__compiler_pellesc(C)

set(CMAKE_C_OUTPUT_EXTENSION ".obj")
set(CMAKE_C_VERBOSE_FLAG "-V2")

include (Compiler/CMakeCommonCompilerMacros)

set(CMAKE_C90_STANDARD_COMPILE_OPTION "") # No C90-only mode.
set(CMAKE_C90_STANDARD__HAS_FULL_SUPPORT ON)
set(CMAKE_C99_STANDARD_COMPILE_OPTION -std=c99)
set(CMAKE_C99_EXTENSION_COMPILE_OPTION -std=c99 -Zx)
set(CMAKE_C99_STANDARD__HAS_FULL_SUPPORT ON)
set(CMAKE_C11_STANDARD_COMPILE_OPTION -std=c11)
set(CMAKE_C11_EXTENSION_COMPILE_OPTION -std=c11 -Zx)
set(CMAKE_C11_STANDARD__HAS_FULL_SUPPORT ON)
set(CMAKE_C17_STANDARD_COMPILE_OPTION -std=c17)
set(CMAKE_C17_EXTENSION_COMPILE_OPTION -std=c17 -Zx)
set(CMAKE_C_STANDARD_LATEST 17)

if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 12)
  set(CMAKE_C23_STANDARD_COMPILE_OPTION -std=c23)
  set(CMAKE_C23_EXTENSION_COMPILE_OPTION -std=c23 -Zx)
  set(CMAKE_C_STANDARD_LATEST 23)
endif()

__compiler_check_default_language_standard(C 9.0 17 12.0 23)
