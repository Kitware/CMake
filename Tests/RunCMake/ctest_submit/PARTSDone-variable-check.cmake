# Verify that set(CTEST_SUBMIT_PARTS Done) caused only Done.xml to be submitted.
# The upload attempt appears in stderr as "Error when uploading file: .../Done.xml".
if(NOT actual_stderr MATCHES "Error when uploading file:[^\n]*/Done\\.xml")
  set(RunCMake_TEST_FAILED
    "Expected stderr to contain an upload attempt for Done.xml.\nActual stderr:\n${actual_stderr}")
endif()

# No other XML part files should have been uploaded.
foreach(unexpected IN ITEMS Configure Build Test Update Coverage MemCheck Notes Upload)
  if(actual_stderr MATCHES "Error when uploading file:[^\n]*/${unexpected}\\.xml")
    string(APPEND RunCMake_TEST_FAILED
      "\nUnexpected upload attempt for ${unexpected}.xml found in stderr.")
  endif()
endforeach()
