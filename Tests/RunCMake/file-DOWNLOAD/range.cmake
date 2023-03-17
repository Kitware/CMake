include(common.cmake)

set(file ${CMAKE_CURRENT_BINARY_DIR}/output1.png)
file_download(RANGE_START 0 EXPECTED_MD5 dbd330d52f4dbd60115d4191904ded92)

set(file ${CMAKE_CURRENT_BINARY_DIR}/output2.png)
file_download(RANGE_END 50 EXPECTED_MD5 8592e5665b839b5d23825dc84c135b61)

set(file ${CMAKE_CURRENT_BINARY_DIR}/output3.png)
file_download(RANGE_START 10 RANGE_END 50 EXPECTED_MD5 36cd52681e6c6c8fef85fcd9e86fc30d)

set(file ${CMAKE_CURRENT_BINARY_DIR}/output4.png)
file_download(RANGE_START 0  RANGE_END 50
              RANGE_START 60 RANGE_END 100
              EXPECTED_MD5 c5c9e74e82d493dd901eecccd659cebc)
