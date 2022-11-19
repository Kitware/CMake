cmake_policy(SET CMP0141 NEW)
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "")
string(APPEND CMAKE_C_FLAGS_DEBUG_INIT " -Zi")
include(PchReuseFrom-common.cmake)
