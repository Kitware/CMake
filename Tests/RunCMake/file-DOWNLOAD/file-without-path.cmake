include(common.cmake)

set(file_orig "${file}")
cmake_path(GET file_orig FILENAME file)

file_download()

if(NOT EXISTS "${file_orig}")
  message(FATAL_ERROR "file not downloaded to expected path:\n ${file_orig}")
endif()
