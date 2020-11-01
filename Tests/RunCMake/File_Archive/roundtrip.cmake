foreach(parameter OUTPUT_NAME ARCHIVE_FORMAT)
  if(NOT DEFINED ${parameter})
    message(FATAL_ERROR "missing required parameter ${parameter}")
  endif()
endforeach()

set(COMPRESS_DIR compress_dir)
set(FULL_COMPRESS_DIR ${CMAKE_CURRENT_BINARY_DIR}/${COMPRESS_DIR})

set(DECOMPRESS_DIR decompress_dir)
set(FULL_DECOMPRESS_DIR ${CMAKE_CURRENT_BINARY_DIR}/${DECOMPRESS_DIR})

set(FULL_OUTPUT_NAME ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_NAME})

set(CHECK_FILES
  "f1.txt"
  "d1/f1.txt"
  "d 2/f1.txt"
  "d + 3/f1.txt"
  "d_4/f1.txt"
  "d-4/f1.txt"
  "My Special Directory/f1.txt"
)

foreach(file ${CHECK_FILES})
  configure_file(${CMAKE_CURRENT_LIST_FILE} ${FULL_COMPRESS_DIR}/${file} COPYONLY)
endforeach()

if(UNIX)
  execute_process(COMMAND ln -sf f1.txt ${FULL_COMPRESS_DIR}/d1/f2.txt)
  list(APPEND CHECK_FILES "d1/f2.txt")
endif()

file(REMOVE ${FULL_OUTPUT_NAME})
file(REMOVE_RECURSE ${FULL_DECOMPRESS_DIR})
file(MAKE_DIRECTORY ${FULL_DECOMPRESS_DIR})

file(ARCHIVE_CREATE
  OUTPUT ${FULL_OUTPUT_NAME}
  FORMAT "${ARCHIVE_FORMAT}"
  COMPRESSION "${COMPRESSION_TYPE}"
  VERBOSE
  PATHS ${COMPRESS_DIR})

file(ARCHIVE_EXTRACT
  INPUT ${FULL_OUTPUT_NAME}
  ${DECOMPRESSION_OPTIONS}
  DESTINATION ${FULL_DECOMPRESS_DIR}
  VERBOSE)

if(CUSTOM_CHECK_FILES)
  set(CHECK_FILES ${CUSTOM_CHECK_FILES})
endif()

foreach(file ${CHECK_FILES})
  set(input ${FULL_COMPRESS_DIR}/${file})
  set(output ${FULL_DECOMPRESS_DIR}/${COMPRESS_DIR}/${file})

  if(NOT EXISTS ${input})
    message(SEND_ERROR "Cannot find input file ${output}")
  endif()

  if(NOT EXISTS ${output})
    message(SEND_ERROR "Cannot find output file ${output}")
  endif()

  file(MD5 ${input} input_md5)
  file(MD5 ${output} output_md5)

  if(NOT input_md5 STREQUAL output_md5)
    message(SEND_ERROR "Files \"${input}\" and \"${output}\" are different")
  endif()
endforeach()

foreach(file ${NOT_EXISTING_FILES_CHECK})
  set(output ${FULL_DECOMPRESS_DIR}/${COMPRESS_DIR}/${file})

  if(EXISTS ${output})
    message(SEND_ERROR "File ${output} exists but it shouldn't")
  endif()
endforeach()

function(check_magic EXPECTED)
  file(READ ${FULL_OUTPUT_NAME} ACTUAL
    ${ARGN}
  )

  if(NOT ACTUAL STREQUAL EXPECTED)
    message(FATAL_ERROR
      "Actual [${ACTUAL}] does not match expected [${EXPECTED}]")
  endif()
endfunction()


function(check_compression_level COMPRESSION_LEVEL)
  file(ARCHIVE_CREATE
    OUTPUT "${FULL_OUTPUT_NAME}_compression_level"
    FORMAT "${ARCHIVE_FORMAT}"
    COMPRESSION_LEVEL ${COMPRESSION_LEVEL}
    COMPRESSION "${COMPRESSION_TYPE}"
    VERBOSE
    PATHS ${COMPRESS_DIR})

  file(ARCHIVE_EXTRACT
    INPUT "${FULL_OUTPUT_NAME}_compression_level"
    ${DECOMPRESSION_OPTIONS}
    DESTINATION ${FULL_DECOMPRESS_DIR}
    VERBOSE)
endfunction()
