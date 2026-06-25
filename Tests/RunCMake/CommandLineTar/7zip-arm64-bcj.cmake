set(ARCHIVE "${CMAKE_CURRENT_LIST_DIR}/7zip-arm64-bcj.7z")
set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/extract")
set(OUTPUT_FILE "${OUTPUT_DIR}/payload.bin")
set(EXPECTED_HEX "1f2003d500000094000000141f2003d5")

file(REMOVE_RECURSE "${OUTPUT_DIR}")
file(MAKE_DIRECTORY "${OUTPUT_DIR}")

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E tar xvf "${ARCHIVE}"
  WORKING_DIRECTORY "${OUTPUT_DIR}"
  RESULT_VARIABLE result
  OUTPUT_VARIABLE stdout
  ERROR_VARIABLE stderr
)

if(NOT result EQUAL 0)
  message(FATAL_ERROR
    "Failed to extract ARM64 BCJ 7z archive:\n"
    "result: ${result}\n"
    "stdout: ${stdout}\n"
    "stderr: ${stderr}"
  )
endif()

if(NOT EXISTS "${OUTPUT_FILE}")
  message(FATAL_ERROR "Expected file was not extracted:\n  ${OUTPUT_FILE}")
endif()

file(READ "${OUTPUT_FILE}" actual_hex HEX)
if(NOT actual_hex STREQUAL EXPECTED_HEX)
  message(FATAL_ERROR
    "Extracted file content mismatch:\n"
    "expected: ${EXPECTED_HEX}\n"
    "actual: ${actual_hex}"
  )
endif()
