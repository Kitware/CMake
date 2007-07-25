IF(CMAKE_BINARY_DIR)
  MESSAGE(FATAL_ERROR "CPackDeb.cmake may only be used by CPack internally.")
ENDIF(CMAKE_BINARY_DIR)

IF(NOT UNIX)
  MESSAGE(FATAL_ERROR "CPackDeb.cmake may only be used under UNIX.")
ENDIF(NOT UNIX)

FIND_PROGRAM(AR_EXECUTABLE ar)

