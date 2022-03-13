include(${CMAKE_CURRENT_LIST_DIR}/mtime-tests.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

foreach(file ${CHECK_FILES})
  file(TIMESTAMP ${FULL_DECOMPRESS_DIR}/${COMPRESS_DIR}/${file} MTIME UTC)
  if(NOT MTIME STREQUAL ARCHIVE_MTIME_RFC3339)
    message(FATAL_ERROR
      "Extracted timestamp ${MTIME} does not match expected ${ARCHIVE_MTIME_RFC3339}")
  endif()
endforeach()
