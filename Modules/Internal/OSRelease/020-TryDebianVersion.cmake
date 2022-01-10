# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# Author: Alex Turbov

if(NOT EXISTS "${CMAKE_SYSROOT}/etc/debian_version")
  return()
endif()

# Get the first string only
file(
    STRINGS "${CMAKE_SYSROOT}/etc/debian_version" CMAKE_GET_OS_RELEASE_FALLBACK_CONTENT
    LIMIT_COUNT 1
  )

#
# Example:
#   6.0.10          # Old debian
#   wheezy/sid      # Ubuntu
#
if(CMAKE_GET_OS_RELEASE_FALLBACK_CONTENT MATCHES "[0-9]+(\.[0-9]+)*")

  set(CMAKE_GET_OS_RELEASE_FALLBACK_RESULT_NAME Debian)
  set(CMAKE_GET_OS_RELEASE_FALLBACK_RESULT_ID debian)
  set(CMAKE_GET_OS_RELEASE_FALLBACK_RESULT_VERSION ${CMAKE_GET_OS_RELEASE_FALLBACK_CONTENT})
  set(CMAKE_GET_OS_RELEASE_FALLBACK_RESULT_VERSION_ID ${CMAKE_GET_OS_RELEASE_FALLBACK_CONTENT})

  list(
      APPEND CMAKE_GET_OS_RELEASE_FALLBACK_RESULT
      CMAKE_GET_OS_RELEASE_FALLBACK_RESULT_NAME
      CMAKE_GET_OS_RELEASE_FALLBACK_RESULT_ID
      CMAKE_GET_OS_RELEASE_FALLBACK_RESULT_VERSION
      CMAKE_GET_OS_RELEASE_FALLBACK_RESULT_VERSION_ID
    )

endif()

unset(CMAKE_GET_OS_RELEASE_FALLBACK_CONTENT)
