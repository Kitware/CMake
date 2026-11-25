set(ubsan_flags "-fsanitize=undefined,integer,implicit-conversion,float-divide-by-zero,local-bounds -fsanitize-ignorelist=\"${CMAKE_CURRENT_LIST_DIR}/fedora45_ubsan.ignorelist\" -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS "${ubsan_flags}" CACHE STRING "")
set(CMAKE_CXX_FLAGS "${ubsan_flags}" CACHE STRING "")
unset(ubsan_flags)
set(CMake_QT_MAJOR_VERSION "5" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_fedora45_common.cmake")
