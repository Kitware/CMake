#***************************************************************************
#                                  _   _ ____  _
#  Project                     ___| | | |  _ \| |
#                             / __| | | | |_) | |
#                            | (__| |_| |  _ <| |___
#                             \___|\___/|_| \_\_____|
#
# Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at https://curl.se/docs/copyright.html.
#
# You may opt to use, copy, modify, merge, publish, distribute and/or sell
# copies of the Software, and permit persons to whom the Software is
# furnished to do so, under the terms of the COPYING file.
#
# This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
# KIND, either express or implied.
#
# SPDX-License-Identifier: curl
#
###########################################################################
include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(CheckTypeSize)

macro(add_header_include check header)
  if(${check})
    set(_source_epilogue "${_source_epilogue}
      #include <${header}>")
  endif()
endmacro()

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if(NOT DEFINED HAVE_STRUCT_SOCKADDR_STORAGE)
  set(CMAKE_EXTRA_INCLUDE_FILES)
  if(WIN32)
    set(CMAKE_EXTRA_INCLUDE_FILES "winsock2.h")
    set(CMAKE_REQUIRED_DEFINITIONS "-DWIN32_LEAN_AND_MEAN")
    set(CMAKE_REQUIRED_LIBRARIES "ws2_32")
  elseif(HAVE_SYS_SOCKET_H)
    set(CMAKE_EXTRA_INCLUDE_FILES "sys/socket.h")
  endif()
  check_type_size("struct sockaddr_storage" SIZEOF_STRUCT_SOCKADDR_STORAGE)
  set(HAVE_STRUCT_SOCKADDR_STORAGE ${HAVE_SIZEOF_STRUCT_SOCKADDR_STORAGE})
endif()

if(NOT WIN32)
  set(_source_epilogue "#undef inline")
  add_header_include(HAVE_SYS_TYPES_H "sys/types.h")
  add_header_include(HAVE_SYS_SOCKET_H "sys/socket.h")
  check_c_source_compiles("${_source_epilogue}
    int main(void)
    {
      int flag = MSG_NOSIGNAL;
      (void)flag;
      return 0;
    }" HAVE_MSG_NOSIGNAL)
endif()

set(_source_epilogue "#undef inline")
add_header_include(HAVE_SYS_TIME_H "sys/time.h")
check_c_source_compiles("${_source_epilogue}
  #include <time.h>
  int main(void)
  {
    struct timeval ts;
    ts.tv_sec  = 0;
    ts.tv_usec = 0;
    (void)ts;
    return 0;
  }" HAVE_STRUCT_TIMEVAL)

unset(CMAKE_TRY_COMPILE_TARGET_TYPE)

if(NOT CMAKE_CROSSCOMPILING AND NOT APPLE)
  set(_source_epilogue "#undef inline")
  add_header_include(HAVE_SYS_POLL_H "sys/poll.h")
  add_header_include(HAVE_POLL_H "poll.h")
  check_c_source_runs("${_source_epilogue}
    #include <stdlib.h>
    #include <sys/time.h>
    int main(void)
    {
      if(0 != poll(0, 0, 10)) {
        return 1; /* fail */
      }
      else {
        /* detect the 10.12 poll() breakage */
        struct timeval before, after;
        int rc;
        size_t us;

        gettimeofday(&before, NULL);
        rc = poll(NULL, 0, 500);
        gettimeofday(&after, NULL);

        us = (after.tv_sec - before.tv_sec) * 1000000 +
          (after.tv_usec - before.tv_usec);

        if(us < 400000) {
          return 1;
        }
      }
      return 0;
    }" HAVE_POLL_FINE)
endif()

# Detect HAVE_GETADDRINFO_THREADSAFE

if(WIN32)
  set(HAVE_GETADDRINFO_THREADSAFE ${HAVE_GETADDRINFO})
elseif(NOT HAVE_GETADDRINFO)
  set(HAVE_GETADDRINFO_THREADSAFE FALSE)
elseif(APPLE OR
       CMAKE_SYSTEM_NAME STREQUAL "AIX" OR
       CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR
       CMAKE_SYSTEM_NAME STREQUAL "HP-UX" OR
       CMAKE_SYSTEM_NAME STREQUAL "MidnightBSD" OR
       CMAKE_SYSTEM_NAME STREQUAL "NetBSD" OR
       CMAKE_SYSTEM_NAME STREQUAL "SunOS")
  set(HAVE_GETADDRINFO_THREADSAFE TRUE)
elseif(CMAKE_SYSTEM_NAME MATCHES "BSD")
  set(HAVE_GETADDRINFO_THREADSAFE FALSE)
endif()

if(NOT DEFINED HAVE_GETADDRINFO_THREADSAFE)
  set(_source_epilogue "#undef inline")
  add_header_include(HAVE_SYS_SOCKET_H "sys/socket.h")
  add_header_include(HAVE_SYS_TIME_H "sys/time.h")
  add_header_include(HAVE_NETDB_H "netdb.h")
  check_c_source_compiles("${_source_epilogue}
    int main(void)
    {
    #ifdef h_errno
      return 0;
    #else
      force compilation error
    #endif
    }" HAVE_H_ERRNO)

  if(NOT HAVE_H_ERRNO)
    check_c_source_compiles("${_source_epilogue}
      int main(void)
      {
        h_errno = 2;
        return h_errno != 0 ? 1 : 0;
      }" HAVE_H_ERRNO_ASSIGNABLE)

    if(NOT HAVE_H_ERRNO_ASSIGNABLE)
      check_c_source_compiles("${_source_epilogue}
        int main(void)
        {
        #if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200809L)
          return 0;
        #elif defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 700)
          return 0;
        #else
          force compilation error
        #endif
        }" HAVE_H_ERRNO_SBS_ISSUE_7)
    endif()
  endif()

  if(HAVE_H_ERRNO OR HAVE_H_ERRNO_ASSIGNABLE OR HAVE_H_ERRNO_SBS_ISSUE_7)
    set(HAVE_GETADDRINFO_THREADSAFE TRUE)
  endif()
endif()

if(NOT WIN32 AND NOT DEFINED HAVE_CLOCK_GETTIME_MONOTONIC_RAW)
  set(_source_epilogue "#undef inline")
  add_header_include(HAVE_SYS_TYPES_H "sys/types.h")
  add_header_include(HAVE_SYS_TIME_H "sys/time.h")
  check_c_source_compiles("${_source_epilogue}
    #include <time.h>
    int main(void)
    {
      struct timespec ts;
      (void)clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
      return 0;
    }" HAVE_CLOCK_GETTIME_MONOTONIC_RAW)
endif()
