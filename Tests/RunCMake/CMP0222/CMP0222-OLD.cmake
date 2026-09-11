cmake_policy(SET CMP0222 OLD)

# Still usable as a value under OLD.  Must come first: the case below is
# fatal.
set(PATH_IS_PREFIX "value")
if(PATH_IS_PREFIX STREQUAL "value")
else()
  message(SEND_ERROR "PATH_IS_PREFIX not usable as a value under OLD")
endif()

if("/path1" PATH_IS_PREFIX "/path2")
  message("PATH_IS_PREFIX recognized")
endif()
