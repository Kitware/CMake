cmake_minimum_required(VERSION 3.4)

project(ServerTests)

macro(do_build name)
  make_directory("${CMAKE_BINARY_DIR}/${name}")

  execute_process(COMMAND ${CMAKE_COMMAND} "${CMAKE_SOURCE_DIR}/${name}"
                  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${name}"
  )
endmacro()

macro(do_test bsname req_ res)
  string(REPLACE "\n" "\\n" req ${req_})
  execute_process(COMMAND python
      "${CMAKE_SOURCE_DIR}/daemon-test.py"
      "${CMAKE_COMMAND}"
      "${CMAKE_BINARY_DIR}/${bsname}"
      "${req}"
      "${res}"
    RESULT_VARIABLE test_result)

  if (NOT test_result EQUAL 0)
    message(SEND_ERROR "TEST FAILED")
  endif()
endmacro()

add_subdirectory(bs1tests)
add_subdirectory(bs2tests)

add_executable(Server empty.cpp)
