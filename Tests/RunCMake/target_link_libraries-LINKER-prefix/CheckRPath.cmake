file(RPATH_CHECK FILE "${file}" RPATH "${rpath}")
if(NOT EXISTS "${file}")
  message(FATAL_ERROR "RPATH for ${file} did not contain the expected value")
endif()
