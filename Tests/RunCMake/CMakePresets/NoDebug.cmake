include(${CMAKE_CURRENT_LIST_DIR}/DebugBase.cmake)
if(EXISTS "${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/CMakeLists.txt")
  message(SEND_ERROR "Not debugging try_compile() did not work")
endif()
