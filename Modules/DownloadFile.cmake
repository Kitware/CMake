#
# Use 'cmake -Dremote=${url} -Dlocal=${filename} -Dtimeout=${seconds}
#   -P DownloadFile.cmake' to call this script...
#
if(NOT DEFINED remote)
  message(FATAL_ERROR "error: required variable 'remote' not defined...")
endif()

if(NOT DEFINED local)
  message(FATAL_ERROR "error: required variable 'local' not defined...")
endif()

if(NOT DEFINED timeout)
  set(timeout 30)
endif(NOT DEFINED timeout)

message(STATUS "info: downloading '${remote}'...")
file(DOWNLOAD "${remote}" "${local}" TIMEOUT ${timeout} STATUS status LOG log)

list(GET status 0 status_code)
list(GET status 1 status_string)

if(NOT status_code EQUAL 0)
  message(FATAL_ERROR "error: download of '${remote}' failed
status_code: ${status_code}
status_string: ${status_string}
log: ${log}
")
endif()

message(STATUS "info: done downloading '${remote}'...")
