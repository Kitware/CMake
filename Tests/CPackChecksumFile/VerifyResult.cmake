# prevent older policies from interfearing with this script
message(STATUS "=============================================================================")
message(STATUS "CTEST_FULL_OUTPUT (Avoid ctest truncation of output)")
message(STATUS "")


if (NOT DEFINED ChecksumConf)
  message(FATAL_ERROR "ChecksumConf should be defined")
endif()

if (NOT DEFINED CPackChecksumFile_BINARY_DIR)
  message(FATAL_ERROR "CPackChecksumFile_BINARY_DIR should be defined")
endif()

if (NOT DEFINED CMAKE_CMAKE_COMMAND)
  message(FATAL_ERROR "CMAKE_CMAKE_COMMAND should be defined")
endif()

execute_process(COMMAND ${CMAKE_CMAKE_COMMAND} --build . --target package
                ERROR_VARIABLE CHECKSUM_ERR
                WORKING_DIRECTORY ${CPackChecksumFile_BINARY_DIR})

# Special case of verify test
if(ChecksumConf MATCHES "invalid")
  if(NOT CHECKSUM_ERR MATCHES "CPack Error: Cannot recognize algorithm: c6790w4nv2b12n048...")
    message(FATAL_ERROR "Error output is not that expected!")
  endif()
else()
# Transform to 
  string(TOUPPER ${ChecksumConf} ALGO)
  file(GLOB PACKAGE RELATIVE ${CPackChecksumFile_BINARY_DIR} "*.tar.gz")
  file(GLOB CSUMFILE RELATIVE ${CPackChecksumFile_BINARY_DIR} "*.${ChecksumConf}")
  file(STRINGS ${CSUMFILE} CHSUM_VALUE)
  file(${ALGO} ${PACKAGE} expected_value )
  set(expected_value "${expected_value}  ${PACKAGE}")

  if(NOT expected_value STREQUAL CHSUM_VALUE)
    message(FATAL_ERROR "Generated checksum is not valid! Expected [${expected_value}] Got [${CHSUM_VALUE}]")
  endif()
endif()
