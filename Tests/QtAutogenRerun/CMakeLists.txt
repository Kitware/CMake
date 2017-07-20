cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0071 NEW)
project(QtAutogenRerun)

# Tell find_package(Qt5) where to find Qt.
if(QT_QMAKE_EXECUTABLE)
  get_filename_component(Qt_BIN_DIR "${QT_QMAKE_EXECUTABLE}" PATH)
  get_filename_component(Qt_PREFIX_DIR "${Qt_BIN_DIR}" PATH)
  set(CMAKE_PREFIX_PATH ${Qt_PREFIX_DIR})
endif()

if (QT_TEST_VERSION STREQUAL 4)
  find_package(Qt4 REQUIRED)

  # Include this directory before using the UseQt4 file.
  add_subdirectory(defines_test)

  include(UseQt4)

  set(QT_QTCORE_TARGET Qt4::QtCore)

else()
  if (NOT QT_TEST_VERSION STREQUAL 5)
    message(SEND_ERROR "Invalid Qt version specified.")
  endif()
  find_package(Qt5Widgets REQUIRED)

  set(QT_QTCORE_TARGET Qt5::Core)

  include_directories(${Qt5Widgets_INCLUDE_DIRS})
  set(QT_LIBRARIES Qt5::Widgets)

  if(Qt5_POSITION_INDEPENDENT_CODE AND CMAKE_CXX_COMPILE_OPTIONS_PIC)
    add_definitions(${CMAKE_CXX_COMPILE_OPTIONS_PIC})
  endif()

endif()

# -- Test
# Dummy test to generate clean target
add_executable(dummy dummy.cpp)

# -- Test
# When a file listed in a .qrc file changes the target must be rebuilt
set(timeformat "%Y%j%H%M%S")
set(RCC_DEPENDS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/rccDepends")
set(RCC_DEPENDS_BIN "${CMAKE_CURRENT_BINARY_DIR}/rccDepends")
configure_file(${RCC_DEPENDS_SRC}/res1a.qrc.in ${RCC_DEPENDS_BIN}/res1.qrc COPYONLY)
configure_file(${RCC_DEPENDS_SRC}/res2a.qrc.in ${RCC_DEPENDS_BIN}/res2.qrc.in COPYONLY)
try_compile(RCC_DEPENDS
  "${RCC_DEPENDS_BIN}"
  "${RCC_DEPENDS_SRC}"
  rccDepends
  CMAKE_FLAGS "-DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}"
              "-DQT_TEST_VERSION=${QT_TEST_VERSION}"
              "-DCMAKE_PREFIX_PATH=${Qt_PREFIX_DIR}"
  OUTPUT_VARIABLE output
)
if (NOT RCC_DEPENDS)
  message(SEND_ERROR "Initial build of rccDepends failed. Output: ${output}")
endif()
# Get name and timestamp of the output binary
file(STRINGS "${RCC_DEPENDS_BIN}/target.txt" targetList ENCODING UTF-8)
list(GET targetList 0 rccDependsBin)
file(TIMESTAMP "${rccDependsBin}" timeBegin "${timeformat}")
# Sleep, touch regular qrc input file, rebuild and compare timestamp
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1) # Ensure that the timestamp will change.
execute_process(COMMAND "${CMAKE_COMMAND}" -E touch "${RCC_DEPENDS_BIN}/res1/input.txt")
execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${RCC_DEPENDS_BIN}" RESULT_VARIABLE result)
if (result)
  message(SEND_ERROR "Second build of rccDepends failed.")
endif()
file(TIMESTAMP "${rccDependsBin}" timeStep1 "${timeformat}")
if (NOT timeStep1 GREATER timeBegin)
  message(SEND_ERROR "File (${rccDependsBin}) should have changed in the first step!")
endif()
# Sleep, update regular qrc file, rebuild and compare timestamp
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1) # Ensure that the timestamp will change.
configure_file(${RCC_DEPENDS_SRC}/res1b.qrc.in ${RCC_DEPENDS_BIN}/res1.qrc COPYONLY)
execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${RCC_DEPENDS_BIN}" RESULT_VARIABLE result)
if (result)
  message(SEND_ERROR "Third build of rccDepends failed.")
endif()
file(TIMESTAMP "${rccDependsBin}" timeStep2 "${timeformat}")
if (NOT timeStep2 GREATER timeStep1)
  message(SEND_ERROR "File (${rccDependsBin}) should have changed in the second step!")
endif()
# Sleep, touch regular qrc newly added input file, rebuild and compare timestamp
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1) # Ensure that the timestamp will change.
execute_process(COMMAND "${CMAKE_COMMAND}" -E touch "${RCC_DEPENDS_BIN}/res1/inputAdded.txt")
execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${RCC_DEPENDS_BIN}" RESULT_VARIABLE result)
if (result)
  message(SEND_ERROR "Fourth build of rccDepends failed.")
endif()
file(TIMESTAMP "${rccDependsBin}" timeStep3 "${timeformat}")
if (NOT timeStep3 GREATER timeStep2)
  message(SEND_ERROR "File (${rccDependsBin}) should have changed in the third step!")
endif()
# Sleep, touch generated qrc input file, rebuild and compare timestamp
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1) # Ensure that the timestamp will change.
execute_process(COMMAND "${CMAKE_COMMAND}" -E touch "${RCC_DEPENDS_BIN}/res2/input.txt")
execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${RCC_DEPENDS_BIN}" RESULT_VARIABLE result)
if (result)
  message(SEND_ERROR "Fifth build of rccDepends failed.")
endif()
file(TIMESTAMP "${rccDependsBin}" timeStep4 "${timeformat}")
if (NOT timeStep4 GREATER timeStep3)
  message(SEND_ERROR "File (${rccDependsBin}) should have changed in the fourth step!")
endif()
# Sleep, update generated qrc file, rebuild and compare timestamp
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1) # Ensure that the timestamp will change.
configure_file(${RCC_DEPENDS_SRC}/res2b.qrc.in ${RCC_DEPENDS_BIN}/res2.qrc.in COPYONLY)
execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${RCC_DEPENDS_BIN}" RESULT_VARIABLE result)
if (result)
  message(SEND_ERROR "Sixth build of rccDepends failed.")
endif()
file(TIMESTAMP "${rccDependsBin}" timeStep5 "${timeformat}")
if (NOT timeStep5 GREATER timeStep4)
  message(SEND_ERROR "File (${rccDependsBin}) should have changed in the fitfh step!")
endif()
# Sleep, touch generated qrc newly added input file, rebuild and compare timestamp
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1) # Ensure that the timestamp will change.
execute_process(COMMAND "${CMAKE_COMMAND}" -E touch "${RCC_DEPENDS_BIN}/res2/inputAdded.txt")
execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${RCC_DEPENDS_BIN}" RESULT_VARIABLE result)
if (result)
  message(SEND_ERROR "Seventh build of rccDepends failed.")
endif()
file(TIMESTAMP "${rccDependsBin}" timeStep6 "${timeformat}")
if (NOT timeStep6 GREATER timeStep5)
  message(SEND_ERROR "File (${rccDependsBin}) should have changed in the sixth step!")
endif()


# -- Test
# Ensure a repeated build succeeds when a header containing a QObject changes
set(timeformat "%Y%j%H%M%S")
configure_file(mocRerun/test1a.h.in mocRerun/test1.h COPYONLY)
try_compile(MOC_RERUN
  "${CMAKE_CURRENT_BINARY_DIR}/mocRerun"
  "${CMAKE_CURRENT_SOURCE_DIR}/mocRerun"
  mocRerun
  CMAKE_FLAGS "-DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}"
              "-DQT_TEST_VERSION=${QT_TEST_VERSION}"
              "-DCMAKE_PREFIX_PATH=${Qt_PREFIX_DIR}"
  OUTPUT_VARIABLE output
)
if (NOT MOC_RERUN)
  message(SEND_ERROR "Initial build of mocRerun failed. Output: ${output}")
endif()
# Get name and timestamp of the output binary
file(STRINGS "${CMAKE_CURRENT_BINARY_DIR}/mocRerun/target1.txt" target1List ENCODING UTF-8)
list(GET target1List 0 binFile)
file(TIMESTAMP "${binFile}" timeBegin "${timeformat}")
# Change file content and rebuild
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1)
configure_file(mocRerun/test1b.h.in mocRerun/test1.h COPYONLY)
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/mocRerun"
  RESULT_VARIABLE mocRerun_result
  )
if (mocRerun_result)
  message(SEND_ERROR "Second build of mocRerun failed.")
endif()
# Compare timestamps
file(TIMESTAMP "${binFile}" timeStep1 "${timeformat}")
if (NOT timeStep1 GREATER timeBegin)
  message(SEND_ERROR "File (${binFile}) should have changed in the first step!")
endif()


# -- Test
# Tests Q_PLUGIN_METADATA json file change detection
if (NOT QT_TEST_VERSION STREQUAL 4)
  try_compile(MOC_PLUGIN
    "${CMAKE_CURRENT_BINARY_DIR}/mocPlugin"
    "${CMAKE_CURRENT_SOURCE_DIR}/mocPlugin"
    mocPlugin
    CMAKE_FLAGS "-DQT_TEST_VERSION=${QT_TEST_VERSION}"
                "-DCMAKE_PREFIX_PATH=${Qt_PREFIX_DIR}"
    OUTPUT_VARIABLE output
  )
  if (NOT MOC_PLUGIN)
    message(SEND_ERROR "Initial build of mocPlugin failed. Output: ${output}")
  endif()

  set(timeformat "%Y%j%H%M%S")
  set(mocPlugSrcDir "${CMAKE_CURRENT_SOURCE_DIR}/mocPlugin")
  set(mocPlugBinDir "${CMAKE_CURRENT_BINARY_DIR}/mocPlugin")
  find_library(plAFile "PlugA" PATHS "${mocPlugBinDir}/Debug" "${mocPlugBinDir}" NO_DEFAULT_PATH)
  find_library(plBFile "PlugB" PATHS "${mocPlugBinDir}/Debug" "${mocPlugBinDir}" NO_DEFAULT_PATH)
  find_library(plCFile "PlugC" PATHS "${mocPlugBinDir}/Debug" "${mocPlugBinDir}" NO_DEFAULT_PATH)
  find_library(plDFile "PlugD" PATHS "${mocPlugBinDir}/Debug" "${mocPlugBinDir}" NO_DEFAULT_PATH)

  file(TIMESTAMP "${plAFile}" plABefore "${timeformat}")
  file(TIMESTAMP "${plBFile}" plBBefore "${timeformat}")
  file(TIMESTAMP "${plCFile}" plCBefore "${timeformat}")
  file(TIMESTAMP "${plDFile}" plDBefore "${timeformat}")

  # Ensure that the timestamp will change and change the json files
  execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1)
  configure_file("${mocPlugSrcDir}/jsonIn/StyleD.json" "${mocPlugBinDir}/jsonFiles/StyleC.json")
  configure_file("${mocPlugSrcDir}/jsonIn/StyleC.json" "${mocPlugBinDir}/jsonFiles/sub/StyleD.json")
  execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${mocPlugBinDir}")

  file(TIMESTAMP "${plAFile}" plAAfter "${timeformat}")
  file(TIMESTAMP "${plBFile}" plBAfter "${timeformat}")
  file(TIMESTAMP "${plCFile}" plCAfter "${timeformat}")
  file(TIMESTAMP "${plDFile}" plDAfter "${timeformat}")

  if (plAAfter GREATER plABefore)
    message(SEND_ERROR "file (${plAFile}) should not have changed!")
  endif()
  if (plBAfter GREATER plBBefore)
    message(SEND_ERROR "file (${plBFile}) should not have changed!")
  endif()
  if (NOT plCAfter GREATER plCBefore)
    message(SEND_ERROR "file (${plCFile}) should have changed!")
  endif()
  if (NOT plDAfter GREATER plDBefore)
    message(SEND_ERROR "file (${plDFile}) should have changed!")
  endif()

  # Test custom macro
  file(TIMESTAMP "${plCFile}" plCBefore "${timeformat}")
  file(TIMESTAMP "${plDFile}" plDBefore "${timeformat}")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1)
  configure_file("${mocPlugSrcDir}/jsonIn/StyleD.json" "${mocPlugBinDir}/jsonFiles/StyleC_Custom.json")
  configure_file("${mocPlugSrcDir}/jsonIn/StyleC.json" "${mocPlugBinDir}/jsonFiles/sub/StyleD_Custom.json")
  execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${mocPlugBinDir}")
  file(TIMESTAMP "${plCFile}" plCAfter "${timeformat}")
  file(TIMESTAMP "${plDFile}" plDAfter "${timeformat}")
  if (NOT plCAfter GREATER plCBefore)
    message(SEND_ERROR "file (${plCFile}) should have changed!")
  endif()
  if (NOT plDAfter GREATER plDBefore)
    message(SEND_ERROR "file (${plDFile}) should have changed!")
  endif()

endif()
