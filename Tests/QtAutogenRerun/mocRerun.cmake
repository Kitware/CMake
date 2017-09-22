
set(timeformat "%Y%j%H%M%S")
set(mocRerunSrcDir "${CMAKE_CURRENT_SOURCE_DIR}/mocRerun")
set(mocRerunBinDir "${CMAKE_CURRENT_BINARY_DIR}/mocRerun")

# Initial build
configure_file("${mocRerunSrcDir}/test1a.h.in" "${mocRerunBinDir}/test1.h" COPYONLY)
try_compile(MOC_RERUN
  "${mocRerunBinDir}"
  "${mocRerunSrcDir}"
  mocRerun
  CMAKE_FLAGS "-DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}"
              "-DQT_TEST_VERSION=${QT_TEST_VERSION}"
              "-DCMAKE_PREFIX_PATH=${Qt_PREFIX_DIR}"
  OUTPUT_VARIABLE output
)
if (NOT MOC_RERUN)
  message(SEND_ERROR "Initial build of mocRerun failed. Output: ${output}")
endif()
# Get name of the output binary
file(STRINGS "${mocRerunBinDir}/mocRerun.txt" mocRerunList ENCODING UTF-8)
list(GET mocRerunList 0 mocRerunBin)

message("Changing the header content for a MOC rerun")
# - Acquire binary timestamps before the build
file(TIMESTAMP "${mocRerunBin}" timeBefore "${timeformat}")
# - Ensure that the timestamp will change
# - Change header file content and rebuild
# - Rebuild
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1)
configure_file("${mocRerunSrcDir}/test1b.h.in" "${mocRerunBinDir}/test1.h" COPYONLY)
execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${mocRerunBinDir}" RESULT_VARIABLE result )
if (result)
  message(SEND_ERROR "Second build of mocRerun failed.")
endif()
# - Acquire binary timestamps after the build
file(TIMESTAMP "${mocRerunBin}" timeAfter "${timeformat}")
# - Test if timestamps changed
if (NOT timeAfter GREATER timeBefore)
  message(SEND_ERROR "File (${mocRerunBin}) should have changed!")
endif()


message("Changing nothing for a MOC rerun")
# - Acquire binary timestamps before the build
file(TIMESTAMP "${mocRerunBin}" timeBefore "${timeformat}")
# - Ensure that the timestamp would change
# - Change nothing
# - Rebuild
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1)
execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${mocRerunBinDir}" RESULT_VARIABLE result )
if (result)
  message(SEND_ERROR "Third build of mocRerun failed.")
endif()
# - Acquire binary timestamps after the build
file(TIMESTAMP "${mocRerunBin}" timeAfter "${timeformat}")
# - Test if timestamps changed
if (timeAfter GREATER timeBefore)
  message(SEND_ERROR "File (${mocRerunBin}) should not have changed!")
endif()
