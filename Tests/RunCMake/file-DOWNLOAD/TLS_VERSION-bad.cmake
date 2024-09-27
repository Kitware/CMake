function(download case)
  # URL with semantics like https://tls-v1-1.badssl.com:1011 is provided by caller
  file(DOWNLOAD ${url} ${ARGN} STATUS status LOG log)
  message(STATUS "${case}: ${status}")
  if(case MATCHES "1\\.2$" AND NOT status MATCHES "^(35|60);")
    message("${log}")
  endif()
endfunction()

set(CMAKE_TLS_VERIFY 1)

if(CMAKE_HOST_WIN32 OR CMAKE_HOST_APPLE)
  # The OS-native TLS implementations support TLS 1.1.
  set(TEST_TLSv1_1 1)
else()
  # OpenSSL 3.1+ does not support TLS 1.1 or older without setting
  # the security level to 0, which curl (correctly) does not do.
  # https://openssl-library.org/news/openssl-3.1-notes/index.html#major-changes-between-openssl-30-and-openssl-310-14-mar-2023
  set(TEST_TLSv1_1 0)
endif()

# The default is to require 1.2.
unset(ENV{CMAKE_TLS_VERSION})
unset(CMAKE_TLS_VERSION)
download(def-1.2)

# The environment variable overrides the default.
set(ENV{CMAKE_TLS_VERSION} 1.2)
download(env-1.2)
if(TEST_TLSv1_1)
  set(ENV{CMAKE_TLS_VERSION} 1.1)
  download(env-1.1)
endif()

# The cmake variable overrides the environment variable.
set(ENV{CMAKE_TLS_VERSION} 1.1)
set(CMAKE_TLS_VERSION 1.2)
download(var-1.2)
if(TEST_TLSv1_1)
  set(ENV{CMAKE_TLS_VERSION} 1.2)
  set(CMAKE_TLS_VERSION 1.1)
  download(var-1.1)
endif()

# The explicit argument overrides the cmake variable and the environment variable.
set(ENV{CMAKE_TLS_VERSION} 1.1)
set(CMAKE_TLS_VERSION 1.1)
download(opt-1.2 TLS_VERSION 1.2)
if(TEST_TLSv1_1)
  set(ENV{CMAKE_TLS_VERSION} 1.2)
  set(CMAKE_TLS_VERSION 1.2)
  download(opt-1.1 TLS_VERSION 1.1)
endif()
