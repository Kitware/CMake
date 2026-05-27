enable_language (C CXX)
include(CheckSourceCompiles)

add_library(cxx_iface INTERFACE IMPORTED)
target_compile_features(cxx_iface INTERFACE cxx_std_11)
set(CMAKE_REQUIRED_LIBRARIES cxx_iface)

check_source_compiles(C "int main() {return 0;}" SHOULD_BUILD)
if(NOT SHOULD_BUILD)
  message(SEND_ERROR "Test fail for valid C source.")
endif()
