include(Compiler/Clang)
__compiler_clang(C)

set(CMAKE_C_LINK_OPTIONS_PIE ${CMAKE_C_COMPILE_OPTIONS_PIE} -Xlinker -pie)
set(CMAKE_C_LINK_OPTIONS_NO_PIE -Xlinker -no_pie)

if(NOT CMAKE_C_COMPILER_VERSION VERSION_LESS 4.0)
  set(CMAKE_C90_STANDARD_COMPILE_OPTION "-std=c90")
  set(CMAKE_C90_EXTENSION_COMPILE_OPTION "-std=gnu90")

  set(CMAKE_C99_STANDARD_COMPILE_OPTION "-std=c99")
  set(CMAKE_C99_EXTENSION_COMPILE_OPTION "-std=gnu99")

  set(CMAKE_C11_STANDARD_COMPILE_OPTION "-std=c11")
  set(CMAKE_C11_EXTENSION_COMPILE_OPTION "-std=gnu11")
endif()

__compiler_check_default_language_standard(C 4.0 99)
