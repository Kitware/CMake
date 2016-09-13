# CPack test: Verify work of generation checksum files
#
# In this test, we create a simple package and generate 
# different checksum files for it. To verify result we have to assume 
# what FILE command is working correctly.
cmake_minimum_required(VERSION 2.6)
project(CPackChecksumFile)

# Create the mylibapp application
add_executable(myapp mylibapp.cpp)

# Installing
install(TARGETS myapp
  RUNTIME
  DESTINATION bin
  COMPONENT apps)

# Packing
set(CPACK_PACKAGE_NAME "MyApp")
set(CPACK_PACKAGE_VERSION "1.0.1")
set(CPACK_GENERATOR "TGZ")

if (NOT DEFINED ChecksumConf)
  message(FATAL_ERROR "ChecksumConf should be defined")
endif()

# Setup project specific CPack-time CPack Config file.
configure_file(${CPackChecksumFile_SOURCE_DIR}/CheckSum-${ChecksumConf}.cmake.in
               ${CPackChecksumFile_BINARY_DIR}/CheckSum-${ChecksumConf}.cmake
               @ONLY)
set(CPACK_PROJECT_CONFIG_FILE ${CPackChecksumFile_BINARY_DIR}/CheckSum-${ChecksumConf}.cmake)

# Include CPack to introduce the appropriate targets
include(CPack)
