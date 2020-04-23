cmake_policy(SET CMP0106 OLD)

set(should_find ON)
include(CMP0106-Common.cmake)
if (NOT DEFINED BUILD_DOCUMENTATION)
  message(FATAL_ERROR
    "Cache variables seem to have not been made with a `OLD` policy "
    "setting.")
endif ()
