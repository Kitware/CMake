file(SHA256 "${FILENAME}" hash)
if(NOT hash STREQUAL "${SHA256SUM}")
  message(FATAL_ERROR "Expected hash of ${FILENAME}:\n  ${SHA256SUM}\nActual hash:\n  ${hash}")
endif()
