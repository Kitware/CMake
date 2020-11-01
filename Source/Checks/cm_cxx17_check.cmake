set(CMake_CXX17_BROKEN 0)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|PGI|Intel")
  if(NOT CMAKE_CXX17_STANDARD_COMPILE_OPTION)
    set(CMake_CXX17_WORKS 0)
  endif()
  if(NOT DEFINED CMake_CXX17_WORKS)
    include(${CMAKE_CURRENT_LIST_DIR}/cm_message_checks_compat.cmake)
    cm_message_checks_compat(
      "Checking if compiler supports needed C++17 constructs"
      __checkStart __checkPass __checkFail)
    message(${__checkStart})
    try_compile(CMake_CXX17_WORKS
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_LIST_DIR}/cm_cxx17_check.cpp
      CMAKE_FLAGS -DCMAKE_CXX_STANDARD=17
      OUTPUT_VARIABLE OUTPUT
      )
    if(CMake_CXX17_WORKS AND "${OUTPUT}" MATCHES "error: no member named.*gets.*in the global namespace")
      set_property(CACHE CMake_CXX17_WORKS PROPERTY VALUE 0)
    endif()
    if(CMake_CXX17_WORKS)
      message(${__checkPass} "yes")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Determining if compiler supports needed C++17 constructs passed with the following output:\n"
        "${OUTPUT}\n"
        "\n"
        )
    else()
      message(${__checkFail} "no")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Determining if compiler supports needed C++17 constructs failed with the following output:\n"
        "${OUTPUT}\n"
        "\n"
        )
    endif()
  endif()
  if(NOT CMake_CXX17_WORKS)
    set(CMake_CXX17_BROKEN 1)
  endif()
endif()
