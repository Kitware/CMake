if(NOT CMAKE_GENERATOR MATCHES "Xcode|Ninja")
  set(CMAKE_Swift_COMPILER "")
  return()
endif()

if(NOT DEFINED CMAKE_Swift_COMPILER)
  set(_desc "Looking for a Swift compiler")
  message(STATUS ${_desc})

  file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/CheckSwift)

  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/CheckSwift/CMakeLists.txt"
  "cmake_minimum_required(VERSION 3.14)
project(CheckSwift Swift)
file(WRITE \"\${CMAKE_CURRENT_BINARY_DIR}/result.cmake\"
  \"set(CMAKE_Swift_COMPILER \\\"\${CMAKE_Swift_COMPILER}\\\")\\n\"
  \"set(CMAKE_Swift_COMPILER_VERSION \\\"\${CMAKE_Swift_COMPILER_VERSION}\\\")\\n\"
  \"set(CMAKE_Swift_FLAGS \\\"\${CMAKE_Swift_FLAGS}\\\")\\n\")
")

  if(CMAKE_GENERATOR_INSTANCE)
    set(_D_CMAKE_GENERATOR_INSTANCE "-DCMAKE_GENERATOR_INSTANCE:INTERNAL=${CMAKE_GENERATOR_INSTANCE}")
  else()
    set(_D_CMAKE_GENERATOR_INSTANCE "")
  endif()

  execute_process(WORKING_DIRECTORY
                    ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/CheckSwift
                  COMMAND
                    ${CMAKE_COMMAND} . -G ${CMAKE_GENERATOR}
                                       -A "${CMAKE_GENERATOR_PLATFORM}"
                                       -T "${CMAKE_GENERATOR_TOOLSET}"
                                       ${_D_CMAKE_GENERATOR_INSTANCE}
                  TIMEOUT
                    60
                  OUTPUT_VARIABLE
                    output
                  ERROR_VARIABLE
                    output
                  RESULT_VARIABLE
                    result)

  include(${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/CheckSwift/result.cmake
    OPTIONAL)
  # FIXME: Replace with message(CONFIGURE_LOG) when CMake version is high enough.
  if(CMAKE_Swift_COMPILER AND "${result}" STREQUAL "0")
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "${_desc} passed with the following output:\n"
      "${output}\n")
  else()
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
      "${_desc} failed with the following output:\n"
      "${output}\n")
  endif()

  message(STATUS "${_desc} - ${CMAKE_Swift_COMPILER}")

  set(CMAKE_Swift_COMPILER "${CMAKE_Swift_COMPILER}" CACHE FILEPATH "Swift compiler")
  set(CMAKE_Swift_COMPILER_VERSION "${CMAKE_Swift_COMPILER_VERSION}" CACHE FILEPATH "Swift compiler version")
  set(CMAKE_Swift_FLAGS "${CMAKE_Swift_FLAGS}" CACHE STRING "Swift flags")

  mark_as_advanced(CMAKE_Swift_COMPILER)
  mark_as_advanced(CMAKE_Swift_FLAGS)
endif()
