# Pre-os-release fallback scripts read ${CMAKE_SYSROOT}; FROM_SYSROOT must
# scope-override it so target/host selection is honored, and the original
# CMAKE_SYSROOT must be restored after each call.
cmake_policy(SET CMP0221 NEW)

# Host baseline with no sysroot in effect (the host itself may be CentOS).
cmake_host_system_information(RESULT baseline QUERY DISTRIB_ID)

set(sysroot "${CMAKE_CURRENT_LIST_DIR}/CentOS6")
set(CMAKE_SYSROOT "${sysroot}")

# Target: the CentOS fallback script detects the fixture under CMAKE_SYSROOT.
cmake_host_system_information(RESULT tgt QUERY FROM_SYSROOT ON DISTRIB_ID)
if(NOT tgt STREQUAL "centos")
  message(FATAL_ERROR "FROM_SYSROOT ON fallback read ID '${tgt}', expected 'centos'")
endif()
if(NOT CMAKE_SYSROOT STREQUAL "${sysroot}")
  message(FATAL_ERROR "CMAKE_SYSROOT not restored after target fallback: '${CMAKE_SYSROOT}'")
endif()

# Host: must read the host os-release, not the target fixture.
cmake_host_system_information(RESULT host QUERY FROM_SYSROOT OFF DISTRIB_ID)
if(NOT host STREQUAL baseline)
  message(FATAL_ERROR "FROM_SYSROOT OFF read '${host}', expected host baseline '${baseline}'")
endif()
if(NOT CMAKE_SYSROOT STREQUAL "${sysroot}")
  message(FATAL_ERROR "CMAKE_SYSROOT not restored after host fallback: '${CMAKE_SYSROOT}'")
endif()
