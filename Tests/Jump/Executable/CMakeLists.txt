IF(NOT LIBRARY_OUTPUT_PATH)
  LINK_DIRECTORIES(${Jump_BINARY_DIR}/Library/Static
                   ${Jump_BINARY_DIR}/Library/Shared)
ENDIF(NOT LIBRARY_OUTPUT_PATH)
ADD_EXECUTABLE(jumpExecutable jumpExecutable.cxx)
TARGET_LINK_LIBRARIES(jumpExecutable jumpStatic jumpShared)
