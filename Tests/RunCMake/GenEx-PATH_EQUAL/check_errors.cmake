
function(EXPECT_TRUE output data reference)
  if (NOT output)
    message(SEND_ERROR "'${data}' not equal to '${reference}'")
  endif()
endfunction()

function(EXPECT_FALSE output data reference)
  if (output)
    message(SEND_ERROR "'${data}' equal to '${reference}'")
  endif()
endfunction()
