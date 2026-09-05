enable_language(CXX)

set(info "")
foreach(var
    CMAKE_CXX_COMPILE_FEATURES
    CMAKE_MAKE_PROGRAM
    )
  if(DEFINED ${var})
    string(APPEND info "set(${var} \"${${var}}\")\n")
  endif()
endforeach()

# A compiler can only take part in C++ module dependency scanning if it has a
# scandep rule.  Report it as a flag: the rule itself contains characters that
# would not survive a round trip through this file.
if(CMAKE_CXX_SCANDEP_SOURCE)
  string(APPEND info "set(have_cxx_scandep 1)\n")
else()
  string(APPEND info "set(have_cxx_scandep 0)\n")
endif()

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/info.cmake" "${info}")
