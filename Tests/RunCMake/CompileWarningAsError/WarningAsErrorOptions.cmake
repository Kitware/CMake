# add compile options to warning_options to ensure unused-function throws a warning
# if warning_options is NOT DEFINED, assume compiler doesn't support warning as error
macro(get_warning_options warning_options)
  if (CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang|XLClang|IBMClang|LCC|NVCC|IntelLLVM)$")
    set(${warning_options} "-Wall")
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"
          OR (CMAKE_CXX_COMPILER_ID STREQUAL "Intel" AND CMAKE_CXX_SIMULATE_ID MATCHES "MSVC"))
    set(${warning_options} "-W4")
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(${warning_options} "-w3")
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "XL")
    set(${warning_options} "-qinfo=all")
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "SunPro")
    set(${warning_options} "+w;+w2")
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Fujitsu")
    set(${warning_options} "SHELL:-w 8")
  endif()
endmacro()
