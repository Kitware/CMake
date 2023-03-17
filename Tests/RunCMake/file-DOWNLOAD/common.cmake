if(NOT "${CMAKE_CURRENT_SOURCE_DIR}" MATCHES "^/")
  set(slash /)
endif()
set(url "file://${slash}${CMAKE_CURRENT_SOURCE_DIR}/input.png")
set(file ${CMAKE_CURRENT_BINARY_DIR}/output.png)

function(file_download)
  file(DOWNLOAD "${url}"
    ${file} # leave unquoted
    TIMEOUT 30
    STATUS status
    ${ARGN}
    )
  message(STATUS "status='${status}'")
endfunction()
