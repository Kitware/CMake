# Disable warnings to avoid changing 3rd party code.
if(CMAKE_C_COMPILER_ID MATCHES
  "^(GNU|LCC|Clang|AppleClang|IBMClang|XLClang|XL|VisualAge|SunPro|HP|Intel|IntelLLVM|NVHPC)$")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
elseif(CMAKE_C_COMPILER_ID STREQUAL "PathScale")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -woffall")
endif()

add_library(cmllpkgc STATIC
  llpkgc.c
  llpkgc_file.c
  llpkgc_file__internal.c
  llpkgc_frag.c
  llpkgc_frag__internal.c
  llpkgc_dep.c
  llpkgc_dep__internal.c
  llpkgc_whitespace__internal.c

  llpkgc.h
  llpkgc_types.h
  llpkgc_file.h
  llpkgc_file__internal.h
  llpkgc_frag.h
  llpkgc_frag__internal.h
  llpkgc_dep.h
  llpkgc_dep__internal.h
  llpkgc_whitespace__internal.h
)
