cmake_policy(SET CMP0067 NEW)
enable_language(CXX)

# Isolate the one try_compile below in the error log.
set(CMakeError_log "${CMAKE_BINARY_DIR}/CMakeFiles/CMakeError.log")
file(REMOVE "${CMakeError_log}")

# Add our own -std= flag to the try_compile check.
set(CMAKE_REQUIRED_FLAGS -std=c++11)

# Tell CMP0128 NEW behavior to append a -std= flag (after ours).
if(CMAKE_CXX_EXTENSIONS_DEFAULT)
  set(CMAKE_CXX_EXTENSIONS OFF)
else()
  set(CMAKE_CXX_EXTENSIONS ON)
endif()

include(CheckSourceCompiles)
check_source_compiles(CXX "
${check_cxx_std}
int main()
{
  return 0;
}
" SRC_COMPILED)
if(NOT SRC_COMPILED)
  if(EXISTS "${CMakeError_log}")
    file(READ "${CMakeError_log}" err_log)
  endif()
  message("${err_log}")
endif()
