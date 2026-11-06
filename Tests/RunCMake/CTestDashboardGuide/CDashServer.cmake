# Run a local stand-in for CDash that records each submission.

cmake_policy(SET CMP0140 NEW)

function(start_cdash_server)
  set(url_file "${RunCMake_BINARY_DIR}/server.url")
  set(submit_log "${RunCMake_BINARY_DIR}/submissions.log")
  set(server_log "${RunCMake_BINARY_DIR}/server.log")
  file(REMOVE "${url_file}" "${submit_log}" "${server_log}")

  execute_process(
    COMMAND
      "${Python_EXECUTABLE}"
      "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/CDashServer.py"
      --url-file "${url_file}" --log-file "${submit_log}"
    OUTPUT_FILE "${server_log}"
    ERROR_FILE "${server_log}"
    RESULT_VARIABLE result
    TIMEOUT 30
  )
  if(NOT result EQUAL 0)
    file(READ "${server_log}" output)
    message(FATAL_ERROR "Failed to start the CDash server:\n  ${result}\n${output}")
  endif()
  foreach(i RANGE 1 10)
    if(EXISTS "${url_file}")
      break()
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 1)
  endforeach()
  if(NOT EXISTS "${url_file}")
    message(FATAL_ERROR "Failed to read the CDash server URL from:\n  ${url_file}")
  endif()
  file(READ "${url_file}" server_url)
  return(PROPAGATE server_url submit_log)
endfunction()

# Keep the server from stopping as idle while a long step runs.
function(ping_cdash_server)
  file(DOWNLOAD "${server_url}/ping" STATUS status)
endfunction()

function(stop_cdash_server)
  file(DOWNLOAD "${server_url}/shutdown" STATUS status)
endfunction()
