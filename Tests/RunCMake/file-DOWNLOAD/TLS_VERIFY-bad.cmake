function(download case)
  file(DOWNLOAD https://expired.badssl.com ${ARGN} STATUS status LOG log)
  message(STATUS "${case}: ${status}")
  if(case MATCHES "1$" AND NOT status MATCHES "^(35|60);")
    message("${log}")
  endif()
endfunction()

# The default is OFF.
unset(CMAKE_TLS_VERIFY)
download(def-0)

# The cmake variable overrides the default.
set(CMAKE_TLS_VERIFY 0)
download(var-0)
set(CMAKE_TLS_VERIFY 1)
download(var-1)

# The explicit argument overrides the cmake variable.
set(CMAKE_TLS_VERIFY 1)
download(opt-0 TLS_VERIFY 0)
set(CMAKE_TLS_VERIFY 0)
download(opt-1 TLS_VERIFY 1)
