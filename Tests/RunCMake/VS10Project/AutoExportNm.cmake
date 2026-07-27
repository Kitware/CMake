enable_language(C)

set(CMAKE_NM "C:/cmake-nm-test/tool-nm.exe")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/platform-toolset.txt"
  "${CMAKE_VS_PLATFORM_TOOLSET}")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/AutoExportNm.c"
  "void auto_export_nm(void) {}\n")
add_library(AutoExportNm SHARED
  "${CMAKE_CURRENT_BINARY_DIR}/AutoExportNm.c")
set_property(TARGET AutoExportNm PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS ON)
