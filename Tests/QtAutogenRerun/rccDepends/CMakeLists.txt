cmake_minimum_required(VERSION 3.9)
project(rccDepends CXX)

if (QT_TEST_VERSION STREQUAL 4)
  find_package(Qt4 REQUIRED)
  set(QT_CORE_TARGET Qt4::QtCore)
else()
  if (NOT QT_TEST_VERSION STREQUAL 5)
    message(SEND_ERROR "Invalid Qt version specified.")
  endif()

  find_package(Qt5Core REQUIRED)
  set(QT_CORE_TARGET Qt5::Core)
endif()

# Enable AUTORCC for all targets
set(CMAKE_AUTORCC ON)

# Initial resource files setup
configure_file(resPlain/input.txt.in resPlain/input.txt COPYONLY)
configure_file(resPlain/input.txt.in resPlain/inputAdded.txt COPYONLY)
configure_file(resGen/input.txt.in resGen/input.txt COPYONLY)
configure_file(resGen/input.txt.in resGen/inputAdded.txt COPYONLY)

# Generated qrc file with dependency
add_custom_command(OUTPUT resGen.qrc
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/resGen.qrc.in
  COMMAND ${CMAKE_COMMAND} -E sleep 2
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/resGen.qrc.in ${CMAKE_CURRENT_BINARY_DIR}/resGen.qrc)

# Target that uses a plain .qrc file
add_executable(rccDependsPlain main.cpp ${CMAKE_CURRENT_BINARY_DIR}/resPlain.qrc)
target_link_libraries(rccDependsPlain ${QT_CORE_TARGET})
add_custom_command(TARGET rccDependsPlain POST_BUILD COMMAND
  ${CMAKE_COMMAND} -E echo "$<TARGET_FILE:rccDependsPlain>" > targetPlain.txt)

# Target that uses a GENERATED .qrc file
add_executable(rccDependsGen main.cpp ${CMAKE_CURRENT_BINARY_DIR}/resGen.qrc )
target_link_libraries(rccDependsGen ${QT_CORE_TARGET})
add_custom_command(TARGET rccDependsGen POST_BUILD COMMAND
  ${CMAKE_COMMAND} -E echo "$<TARGET_FILE:rccDependsGen>" > targetGen.txt)
