# Disable warnings to avoid changing 3rd party code.
if(CMAKE_C_COMPILER_ID MATCHES
  "^(GNU|LCC|Clang|AppleClang|IBMClang|XLClang|XL|VisualAge|SunPro|HP|Intel|IntelLLVM|NVHPC)$")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
elseif(CMAKE_C_COMPILER_ID STREQUAL "PathScale")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -woffall")
endif()

add_library(cmllpkgc STATIC llpkgc.c llpkgc__internal.c)
