if(${actual_stderr_var} MATCHES "A required privilege is not held by the client")
  unset(msg)
endif()
