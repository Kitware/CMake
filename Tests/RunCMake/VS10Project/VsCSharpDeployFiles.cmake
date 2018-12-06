enable_language(CSharp)

set(fileNames
  "${CMAKE_CURRENT_BINARY_DIR}/content1.txt"
  "${CMAKE_CURRENT_BINARY_DIR}/content2.txt"
  "${CMAKE_CURRENT_BINARY_DIR}/content3.txt")

foreach(f ${fileNames})
  message(STATUS "touch ${f}")
  file(TOUCH ${f})
endforeach()

set_source_files_properties( "${CMAKE_CURRENT_BINARY_DIR}/content1.txt"
  PROPERTIES
    VS_COPY_TO_OUT_DIR PreserveNewest
)

set_source_files_properties( "${CMAKE_CURRENT_BINARY_DIR}/content2.txt"
  PROPERTIES
    VS_COPY_TO_OUT_DIR Always
)


add_library(foo SHARED
  foo.cs
  ${fileNames}
)
