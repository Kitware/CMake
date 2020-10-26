include(${CMAKE_CURRENT_LIST_DIR}/DebugBase.cmake)
if(NOT EXISTS "${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/CMakeLists.txt")
  message(SEND_ERROR "Debugging try_compile() did not work")
endif()
