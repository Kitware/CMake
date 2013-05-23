include(Compiler/GNU)
__compiler_gnu(CXX)

if (WIN32)
  set(CMAKE_CXX_COMPILE_OPTIONS_VISIBILITY_INLINES_HIDDEN "-fno-keep-inline-dllexport")
else()
  set(CMAKE_CXX_COMPILE_OPTIONS_VISIBILITY_INLINES_HIDDEN "-fvisibility-inlines-hidden")
endif()
