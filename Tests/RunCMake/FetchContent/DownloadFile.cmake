cmake_policy(SET CMP0169 OLD)

include(FetchContent)

# The file hash depends on the line endings used by git
file(MD5 ${CMAKE_CURRENT_LIST_DIR}/dummyFile.txt md5_hash)

FetchContent_Declare(
  t1
  URL ${CMAKE_CURRENT_LIST_DIR}/dummyFile.txt
  URL_HASH MD5=${md5_hash}
  DOWNLOAD_NO_EXTRACT YES
)

FetchContent_Populate(t1)
