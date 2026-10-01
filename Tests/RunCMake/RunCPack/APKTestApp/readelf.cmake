# A stand-in for llvm-readelf, run as `cmake -P readelf.cmake -d <input>`,
# that prints a NEEDED entry for each `needed <library>` line of the input.
math(EXPR last "${CMAKE_ARGC} - 1")
set(input "${CMAKE_ARGV${last}}")
if(NOT EXISTS "${input}")
  message(FATAL_ERROR "No input '${input}'")
endif()
file(STRINGS "${input}" lines REGEX "^needed ")
foreach(line IN LISTS lines)
  string(REGEX REPLACE "^needed " "" library "${line}")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E echo
    " 0x0000000000000001 (NEEDED) Shared library: [${library}]")
endforeach()
