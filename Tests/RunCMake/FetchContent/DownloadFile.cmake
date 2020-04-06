include(FetchContent)

FetchContent_Declare(
  t1
  URL ${CMAKE_CURRENT_LIST_DIR}/dummyFile.txt
  DOWNLOAD_NO_EXTRACT YES
)

FetchContent_Populate(t1)
