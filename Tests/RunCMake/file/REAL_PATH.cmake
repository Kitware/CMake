
if (NOT WIN32 OR CYGWIN)
  file(TOUCH "${CMAKE_CURRENT_BINARY_DIR}/test.txt")
  file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/test.sym")
  file(CREATE_LINK  "test.txt" "${CMAKE_CURRENT_BINARY_DIR}/test.sym" SYMBOLIC)

  file(REAL_PATH "${CMAKE_CURRENT_BINARY_DIR}/test.sym" real_path)
  if (NOT real_path STREQUAL "${CMAKE_CURRENT_BINARY_DIR}/test.txt")
    message(SEND_ERROR "real path is \"${real_path}\", should be \"${CMAKE_CURRENT_BINARY_DIR}/test.txt\"")
  endif()

  file(REAL_PATH "test.sym" real_path BASE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
  if (NOT real_path STREQUAL "${CMAKE_CURRENT_BINARY_DIR}/test.txt")
    message(SEND_ERROR "real path is \"${real_path}\", should be \"${CMAKE_CURRENT_BINARY_DIR}/test.txt\"")
  endif()
endif()


If (WIN32)
  cmake_path(SET HOME_DIR "$ENV{USERPROFILE}")
  if (NOT HOME_DIR)
    cmake_path(SET HOME_DIR "$ENV{HOME}")
  endif()
else()
  set(HOME_DIR "$ENV{HOME}")
endif()

file(REAL_PATH "~" real_path EXPAND_TILDE)
if (NOT real_path STREQUAL "${HOME_DIR}")
  message(SEND_ERROR "real path is \"${real_path}\", should be \"${HOME_DIR}\"")
endif()

file(REAL_PATH "~/test.txt" real_path EXPAND_TILDE)
if (NOT real_path STREQUAL "${HOME_DIR}/test.txt")
  message(SEND_ERROR "real path is \"${real_path}\", should be \"${HOME_DIR}/test.txt\"")
endif()

if (WIN32)
  cmake_policy(SET CMP0139 NEW)

  set(in "${CMAKE_CURRENT_BINARY_DIR}/AbC.TxT")

  file(REMOVE "${in}")
  file(TOUCH "${in}")

  string(TOLOWER "${in}" low)
  file(REAL_PATH "${low}" out)

  if(NOT "${out}" PATH_EQUAL "${in}")
    message(SEND_ERROR "real path is \"${out}\", should be \"${in}\"")
  endif()
endif()
