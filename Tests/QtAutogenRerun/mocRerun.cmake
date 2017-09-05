
set(timeformat "%Y%j%H%M%S")
set(mocRerunSrcDir "${CMAKE_CURRENT_SOURCE_DIR}/mocRerun")
set(mocRerunBinDir "${CMAKE_CURRENT_BINARY_DIR}/mocRerun")

# Initial build
configure_file(mocRerun/test1a.h.in mocRerun/test1.h COPYONLY)
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
file(STRINGS "${mocRerunBinDir}/target1.txt" target1List ENCODING UTF-8)
list(GET target1List 0 binFile)

file(TIMESTAMP "${binFile}" timeBegin "${timeformat}")

# Change header file content and rebuild
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1)
configure_file(mocRerun/test1b.h.in mocRerun/test1.h COPYONLY)
execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${mocRerunBinDir}" RESULT_VARIABLE result )
if (result)
  message(SEND_ERROR "Second build of mocRerun failed.")
endif()

file(TIMESTAMP "${binFile}" timeStep1 "${timeformat}")

if (NOT timeStep1 GREATER timeBegin)
  message(SEND_ERROR "File (${binFile}) should have changed in the first step!")
endif()
