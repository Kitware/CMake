function(download case)
  # URL with semantics like https://expired.badssl.com is provided by caller
  file(DOWNLOAD ${url} ${ARGN} STATUS status LOG log)
  message(STATUS "${case}: ${status}")
  if(case MATCHES "1$" AND NOT status MATCHES "^(35|60);")
    message("${log}")
  endif()
endfunction()

# The default is OFF.
unset(ENV{CMAKE_TLS_VERIFY})
unset(CMAKE_TLS_VERIFY)
download(def-0)

# The environment variable overrides the default.
set(ENV{CMAKE_TLS_VERIFY} 0)
download(env-0)
set(ENV{CMAKE_TLS_VERIFY} 1)
download(env-1)

# The cmake variable overrides the environment variable.
set(ENV{CMAKE_TLS_VERIFY} 1)
set(CMAKE_TLS_VERIFY 0)
download(var-0)
set(ENV{CMAKE_TLS_VERIFY} 0)
set(CMAKE_TLS_VERIFY 1)
download(var-1)

# The explicit argument overrides the cmake variable and the environment variable.
set(ENV{CMAKE_TLS_VERIFY} 1)
set(CMAKE_TLS_VERIFY 1)
download(opt-0 TLS_VERIFY 0)
set(ENV{CMAKE_TLS_VERIFY} 0)
set(CMAKE_TLS_VERIFY 0)
download(opt-1 TLS_VERIFY 1)
