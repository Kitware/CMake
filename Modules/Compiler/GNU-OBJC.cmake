include(Compiler/GNU)
__compiler_gnu(OBJC)

if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.2)
  set(CMAKE_CXX_COMPILE_OPTIONS_VISIBILITY_INLINES_HIDDEN "-fvisibility-inlines-hidden")
endif()
