include(${CMAKE_CURRENT_LIST_DIR}/mtime-tests.cmake)

set(DECOMPRESSION_OPTIONS --touch)

include(${CMAKE_CURRENT_LIST_DIR}/roundtrip.cmake)

foreach(file ${CHECK_FILES})
  file(TIMESTAMP ${FULL_DECOMPRESS_DIR}/${COMPRESS_DIR}/${file} MTIME UTC)
  if(MTIME STREQUAL ARCHIVE_MTIME_RFC3339)
    message(FATAL_ERROR
      "File has unexpected timestamp ${MTIME}")
  endif()
endforeach()
