
cmake_policy(SET CMP0083 NEW)

include (CheckPIESupported)

check_pie_supported()

if (CMAKE_CXX_LINK_PIE_SUPPORTED)
  file(WRITE "${PIE_SUPPORTED}" "\nset(PIE_SUPPORTED TRUE)\n")
else()
  file(WRITE "${PIE_SUPPORTED}" "\nset(PIE_SUPPORTED FALSE)\n")
endif()

if (CMAKE_CXX_LINK_NO_PIE_SUPPORTED)
  file(APPEND "${PIE_SUPPORTED}" "\nset(NO_PIE_SUPPORTED TRUE)\n")
else()
  file(APPEND "${PIE_SUPPORTED}" "\nset(NO_PIE_SUPPORTED FALSE)\n")
endif()
